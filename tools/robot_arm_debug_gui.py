import math
import queue
import re
import threading
import time
import tkinter as tk
from tkinter import messagebox
from tkinter import ttk

import serial
from serial import SerialException
from serial.tools import list_ports

ARM_BASE_HEIGHT_MM = 60.0
ARM_SHOULDER_LENGTH_MM = 95.0
ARM_FOREARM_LENGTH_MM = 90.0
ARM_TOOL_LENGTH_MM = 55.0
ARM_DISTAL_LENGTH_MM = ARM_FOREARM_LENGTH_MM + ARM_TOOL_LENGTH_MM

LEFT_RIGHT_MIN_PULSE = 1100
LEFT_RIGHT_MAX_PULSE = 1700
FORE_AFT_MIN_PULSE = 1000
FORE_AFT_MAX_PULSE = 1600
GRIPPER_LIFT_MIN_PULSE = 700
GRIPPER_LIFT_MAX_PULSE = 1200
CLAMP_OPEN_PULSE = 800
CLAMP_CLOSE_PULSE = 1500
CLAMP_DEFAULT_PULSE = (CLAMP_OPEN_PULSE + CLAMP_CLOSE_PULSE) // 2
LEFT_RIGHT_DEFAULT_PULSE = (LEFT_RIGHT_MIN_PULSE + LEFT_RIGHT_MAX_PULSE) // 2
FORE_AFT_DEFAULT_PULSE = (FORE_AFT_MIN_PULSE + FORE_AFT_MAX_PULSE) // 2
GRIPPER_LIFT_DEFAULT_PULSE = (GRIPPER_LIFT_MIN_PULSE + GRIPPER_LIFT_MAX_PULSE) // 2

LEFT_RIGHT_MIN_DEG = -60.0
LEFT_RIGHT_MAX_DEG = 60.0
FORE_AFT_MIN_DEG = 30.0
FORE_AFT_MAX_DEG = 90.0
GRIPPER_LIFT_MIN_DEG = -60.0
GRIPPER_LIFT_MAX_DEG = 0.0
STARTUP_PREVIEW_YAW_DEG = 0.0
STARTUP_PREVIEW_SHOULDER_DEG = 60.0
STARTUP_PREVIEW_GRIPPER_DEG = -30.0
SAFE_WORKSPACE_MIN_Z_MM = 0.0
SAFE_WORKSPACE_SAMPLES_PER_JOINT = 25
WORKSPACE_PREVIEW_SAMPLES_PER_JOINT = 17
WORKSPACE_PREVIEW_QUANTIZATION_MM = 6.0
WORKSPACE_CLICK_PICK_RADIUS_PX = 28.0
WORKSPACE_VIEW_DEFAULT_AZIMUTH_DEG = 0.0
WORKSPACE_VIEW_DEFAULT_ELEVATION = 0.28
WORKSPACE_VIEW_MIN_ELEVATION = 0.18
WORKSPACE_VIEW_MAX_ELEVATION = 0.82
WORKSPACE_PREVIEW_EASING = 0.3
WORKSPACE_AXIS_GUIDE_RATIO = 0.42
WORKSPACE_PROJECTION_PADDING_RATIO = 0.08
WORKSPACE_REACTION_TIME_DEFAULT_MS = 120
WORKSPACE_REACTION_TIME_MIN_MS = 0
WORKSPACE_REACTION_TIME_MAX_MS = 1000
TOF_STATUS_POLL_INTERVAL_DEFAULT_MS = 100
TOF_STATUS_POLL_INTERVAL_MIN_MS = 100
TOF_STATUS_POLL_INTERVAL_MAX_MS = 2000
LOG_QUEUE_POLL_INTERVAL_MS = 33

PREVIEW_CANVAS_WIDTH = 760
PREVIEW_CANVAS_HEIGHT = 640

STATUS_POSE_PATTERN = re.compile(
    r"^\[status\] estimated_pose x=(-?\d+) y=(-?\d+) z=(-?\d+) mm q_deg yaw=(-?\d+) shoulder=(-?\d+) gripper=(-?\d+) pulses clamp=(\d+) fore_aft=(\d+) gripper_lift=(\d+) left_right=(\d+)$"
)
STATUS_MOTION_PATTERN = re.compile(
    r"^\[status\] motion=([a-z_]+) remaining=(\d+) ms target_pulses clamp=(\d+) fore_aft=(\d+) gripper_lift=(\d+) left_right=(\d+)$"
)
STATUS_CARTESIAN_PATTERN = re.compile(
    r"^\[status\] cartesian target_xyz x=(-?\d+) y=(-?\d+) z=(-?\d+) mm elbow=([a-z]+)$"
)
STATUS_OBSTACLE_PATTERN = re.compile(
    r"^\[status\] obstacle=([a-z_]+) range=(\d+) mm threshold=(\d+) mm clear=(\d+) mm device=([a-z\-]+)$"
)
STATUS_IMU_PATTERN = re.compile(
    r"^\[status\] imu=([a-z_]+) pitch=(-?\d+) roll=(-?\d+) gyro_mag=(\d+) temp_dc=(-?\d+)$"
)
SAFETY_OBSTACLE_PATTERN = re.compile(
    r"^\[safety\] obstacle (detected|cleared) range=(\d+) mm threshold=(\d+) mm clear=(\d+) mm device=([a-z\-]+)(?: motion=([a-z_]+) action=stop)?$"
)
CMD_HOME_PATTERN = re.compile(
    r"^\[cmd_home\] queued (?:safe startup|default midpoint) pose duration=(\d+) ms target clamp=(\d+) fore_aft=(\d+) gripper_lift=(\d+) left_right=(\d+)$"
)
CMD_PWM_PATTERN = re.compile(
    r"^\[cmd_pwm\] queued request clamp=(-?\d+) fore_aft=(-?\d+) gripper_lift=(-?\d+) left_right=(-?\d+) us target clamp=(\d+) fore_aft=(\d+) gripper_lift=(\d+) left_right=(\d+) duration=(\d+) ms$"
)
CMD_XYZ_CARTESIAN_PATTERN = re.compile(
    r"^\[cmd_xyz\] cartesian queued start=(-?\d+),(-?\d+),(-?\d+) target=(-?\d+),(-?\d+),(-?\d+) mm elbow=([a-z]+).+ duration=(\d+) ms$"
)
CMD_XYZ_REJECT_PATTERN = re.compile(
    r"^\[cmd_xyz\] target=(-?\d+),(-?\d+),(-?\d+) mm rejected status=(.+)$"
)
CMD_XYZ_FALLBACK_PATTERN = re.compile(
    r"^\[cmd_xyz\] current pose estimate unavailable, fallback_to_joint_space target=(-?\d+),(-?\d+),(-?\d+) mm elbow=([a-z]+).+ target_pulses fore_aft=(\d+) gripper_lift=(\d+) left_right=(\d+) duration=(\d+) ms$"
)
CMD_USAGE_PATTERN = re.compile(r"^\[(cmd_[a-z]+)\] usage: (.+)$")
CMD_UNKNOWN_PATTERN = re.compile(r"^\[cmd\] unknown command: (.+)$")


class RobotArmDebugGui(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title("Robot Arm Debug Tool")
        self.geometry("1320x1080")
        self.minsize(960, 820)

        self._workspace_view_azimuth_deg = WORKSPACE_VIEW_DEFAULT_AZIMUTH_DEG
        self._workspace_view_elevation = WORKSPACE_VIEW_DEFAULT_ELEVATION
        self.workspace_bounds = self._estimate_workspace_bounds()
        self.preview_bounds = self._estimate_preview_bounds()
        self.workspace_preview_points = self._sample_workspace_preview_points()
        self.workspace_projection_bounds = self._estimate_workspace_projection_bounds(self.workspace_preview_points)
        default_x_mm = self._guide_axis_midpoint("x")
        default_y_mm = self._guide_axis_midpoint("y")
        default_z_mm = self._guide_axis_midpoint("z")

        self.port_var = tk.StringVar()
        self.connection_var = tk.StringVar(value="Disconnected")
        self.connect_button_var = tk.StringVar(value="Connect")
        self.log_pause_button_var = tk.StringVar(value="Pause Log")
        self.workspace_interaction_mode_var = tk.StringVar(value="Hold Preview / Release Send")
        self.workspace_reaction_time_var = tk.StringVar(value=str(WORKSPACE_REACTION_TIME_DEFAULT_MS))
        self.status_poll_interval_var = tk.StringVar(value=str(TOF_STATUS_POLL_INTERVAL_DEFAULT_MS))

        self.x_var = tk.StringVar(value=str(default_x_mm))
        self.y_var = tk.StringVar(value=str(default_y_mm))
        self.z_var = tk.StringVar(value=str(default_z_mm))

        self.clamp_var = tk.IntVar(value=CLAMP_DEFAULT_PULSE)
        self.fore_aft_var = tk.IntVar(value=FORE_AFT_DEFAULT_PULSE)
        self.gripper_lift_var = tk.IntVar(value=GRIPPER_LIFT_DEFAULT_PULSE)
        self.left_right_var = tk.IntVar(value=LEFT_RIGHT_DEFAULT_PULSE)

        self.preview_mode_var = tk.StringVar(value="Startup preview shows a lamp-like display pose. PWM defaults still start at the midpoint of each range.")
        self.pose_summary_var = tk.StringVar(value="Local pose estimate pending")
        self.target_summary_var = tk.StringVar(value=f"Local target x={default_x_mm} y={default_y_mm} z={default_z_mm} mm")
        self.preview_legend_var = tk.StringVar(
            value="Orange shoulder housing = Fore front/back joint. Blue elbow housing = Gripper up/down joint. Cyan cloud = reachable 3D workspace. Choose hold-preview or live-follow mode."
        )
        self.x_range_var = tk.StringVar(value=self._format_axis_range_text("x"))
        self.y_range_var = tk.StringVar(value=self._format_axis_range_text("y"))
        self.z_range_var = tk.StringVar(value=self._format_axis_range_text("z"))
        self.xyz_range_note_var = tk.StringVar(value=self._format_workspace_note_text())
        self.xyz_validation_var = tk.StringVar(value="Ready: XYZ is inside the local guide range.")
        self.fw_command_status_var = tk.StringVar(value="FW command result: waiting for command")
        self.fw_command_echo_var = tk.StringVar(value="FW echo: no command acknowledgement yet")
        self.firmware_motion_var = tk.StringVar(value="Firmware motion idle | not connected")
        self.firmware_pose_var = tk.StringVar(value="Firmware pose unavailable")
        self.firmware_target_var = tk.StringVar(value="Firmware target unavailable")
        self.firmware_obstacle_var = tk.StringVar(value="Obstacle monitor unavailable")
        self._obstacle_monitor_state = "unknown"
        self._obstacle_range_mm = 0
        self._obstacle_threshold_mm = 120
        self._obstacle_clear_mm = 140
        self._obstacle_device_name = "no-data"
        self._imu_monitor_state = "unknown"
        self._imu_pitch_deg = 0
        self._imu_roll_deg = 0
        self._imu_gyro_mag_mdps = 0
        self._imu_temp_dc = 0
        self._imu_device_name = "no-data"

        self.serial_port = None
        self._port_mapping = {}
        self.reader_stop_event = threading.Event()
        self.reader_thread = None
        self.status_poll_after_id = None
        self.log_queue = queue.Queue()
        self.log_paused = False
        self.paused_log_lines: list[str] = []
        self.preview_canvas = None
        self.scroll_canvas = None
        self.scrollable_frame = None
        self.scroll_window_id = None
        self.command_frame = None
        self.xyz_frame = None
        self.x_entry = None
        self.y_entry = None
        self.z_entry = None
        self.send_xyz_button = None
        self.fw_command_status_label = None
        self.pwm_frame = None
        self.preview_frame = None
        self.workspace_mode_combo = None
        self.workspace_reaction_time_spinbox = None
        self.status_poll_interval_spinbox = None
        self.firmware_obstacle_label = None
        self.command_layout_mode = None
        self._last_preview_canvas_size = (0, 0)
        self._workspace_click_candidates = []
        self._workspace_panel_bounds = None
        self._workspace_hover_target = None
        self._workspace_drag_active = False
        self._workspace_drag_target = None
        self._workspace_rotate_active = False
        self._workspace_rotate_last_pointer = None
        self._workspace_preview_joint_angles = None
        self._workspace_preview_target_angles = None
        self._workspace_preview_after_id = None
        self._workspace_live_follow_after_id = None
        self._workspace_live_follow_pending_target = None
        self._workspace_last_live_send_monotonic = None
        self._suspend_preview_updates = False
        self._initial_preview_joint_angles = self._create_initial_preview_joint_angles()

        self._build_ui()
        self._install_variable_traces()
        self.workspace_interaction_mode_var.trace_add("write", self._handle_workspace_interaction_mode_change)
        self.status_poll_interval_var.trace_add("write", self._handle_status_poll_interval_change)
        self._update_xyz_entry_states()
        self._refresh_xyz_validation_state()
        self.refresh_ports()
        self._update_workspace_interaction_controls()
        self._update_preview()
        self.after(LOG_QUEUE_POLL_INTERVAL_MS, self._poll_log_queue)
        self.protocol("WM_DELETE_WINDOW", self._handle_close)

    def _build_ui(self) -> None:
        self.columnconfigure(0, weight=1)
        self.rowconfigure(0, weight=1)

        self.scroll_canvas = tk.Canvas(self, highlightthickness=0, bd=0)
        self.scroll_canvas.grid(row=0, column=0, sticky="nsew")

        vertical_scrollbar = ttk.Scrollbar(self, orient="vertical", command=self.scroll_canvas.yview)
        vertical_scrollbar.grid(row=0, column=1, sticky="ns")
        horizontal_scrollbar = ttk.Scrollbar(self, orient="horizontal", command=self.scroll_canvas.xview)
        horizontal_scrollbar.grid(row=1, column=0, sticky="ew")

        self.scroll_canvas.configure(yscrollcommand=vertical_scrollbar.set, xscrollcommand=horizontal_scrollbar.set)
        self.scrollable_frame = ttk.Frame(self.scroll_canvas)
        self.scroll_window_id = self.scroll_canvas.create_window((0, 0), window=self.scrollable_frame, anchor="nw")
        self.scrollable_frame.columnconfigure(0, weight=1)
        self.scrollable_frame.rowconfigure(2, weight=1)
        self.scrollable_frame.bind("<Configure>", self._handle_scrollable_frame_configure)
        self.scroll_canvas.bind("<Configure>", self._handle_scroll_canvas_configure)

        connection_frame = ttk.LabelFrame(self.scrollable_frame, text="Serial Link")
        connection_frame.grid(row=0, column=0, sticky="ew", padx=12, pady=(12, 8))
        connection_frame.columnconfigure(1, weight=1)

        ttk.Label(connection_frame, text="Port").grid(row=0, column=0, padx=8, pady=8, sticky="w")
        self.port_combo = ttk.Combobox(connection_frame, textvariable=self.port_var, state="readonly")
        self.port_combo.grid(row=0, column=1, padx=8, pady=8, sticky="ew")
        ttk.Button(connection_frame, text="Refresh", command=self.refresh_ports).grid(row=0, column=2, padx=8, pady=8)
        ttk.Button(connection_frame, textvariable=self.connect_button_var, command=self.toggle_connection).grid(row=0, column=3, padx=8, pady=8)
        ttk.Label(connection_frame, textvariable=self.connection_var).grid(row=0, column=4, padx=8, pady=8, sticky="w")

        self.command_frame = ttk.Frame(self.scrollable_frame)
        self.command_frame.grid(row=1, column=0, sticky="nsew", padx=12, pady=(0, 8))
        self.command_frame.bind("<Configure>", self._handle_command_frame_configure)

        self.xyz_frame = ttk.LabelFrame(self.command_frame, text="XYZ Target")
        self.xyz_frame.grid(row=0, column=0, sticky="nsew", padx=(0, 6))
        for index in range(3):
            self.xyz_frame.columnconfigure(index, weight=1)

        ttk.Label(self.xyz_frame, text="X (mm)").grid(row=0, column=0, padx=8, pady=(10, 4), sticky="w")
        ttk.Label(self.xyz_frame, text="Y (mm)").grid(row=0, column=1, padx=8, pady=(10, 4), sticky="w")
        ttk.Label(self.xyz_frame, text="Z (mm)").grid(row=0, column=2, padx=8, pady=(10, 4), sticky="w")
        self.x_entry = self._create_xyz_entry(self.xyz_frame, self.x_var, 1, 0)
        self.y_entry = self._create_xyz_entry(self.xyz_frame, self.y_var, 1, 1)
        self.z_entry = self._create_xyz_entry(self.xyz_frame, self.z_var, 1, 2)
        ttk.Label(self.xyz_frame, textvariable=self.x_range_var, font=("Segoe UI", 8)).grid(row=2, column=0, padx=8, pady=(0, 2), sticky="w")
        ttk.Label(self.xyz_frame, textvariable=self.y_range_var, font=("Segoe UI", 8)).grid(row=2, column=1, padx=8, pady=(0, 2), sticky="w")
        ttk.Label(self.xyz_frame, textvariable=self.z_range_var, font=("Segoe UI", 8)).grid(row=2, column=2, padx=8, pady=(0, 2), sticky="w")
        tk.Label(self.xyz_frame, textvariable=self.xyz_validation_var, justify="left", anchor="w", fg="#8a1020", bg="#f0f0f0", font=("Segoe UI", 8)).grid(
            row=3, column=0, columnspan=3, padx=8, pady=(0, 4), sticky="ew"
        )
        self.send_xyz_button = ttk.Button(self.xyz_frame, text="Send XYZ", command=self.send_xyz_command)
        self.send_xyz_button.grid(row=4, column=0, padx=8, pady=10, sticky="ew")
        ttk.Button(self.xyz_frame, text="HOME", command=self.send_home_command).grid(row=4, column=1, padx=8, pady=10, sticky="ew")
        ttk.Button(self.xyz_frame, text="STATUS", command=lambda: self.send_command("STATUS")).grid(row=4, column=2, padx=8, pady=10, sticky="ew")

        note = (
            f"{self.xyz_range_note_var.get()}\n"
            "Firmware-side IK and safety limits still make the final accept/reject decision."
        )
        ttk.Label(self.xyz_frame, text=note, justify="left").grid(row=5, column=0, columnspan=3, padx=8, pady=(0, 10), sticky="w")
        ttk.Separator(self.xyz_frame, orient="horizontal").grid(row=6, column=0, columnspan=3, sticky="ew", padx=8, pady=(0, 6))
        ttk.Label(self.xyz_frame, text="FW Command Ack", font=("Segoe UI", 9, "bold")).grid(
            row=7, column=0, columnspan=3, padx=8, pady=(0, 2), sticky="w"
        )
        self.fw_command_status_label = tk.Label(
            self.xyz_frame,
            textvariable=self.fw_command_status_var,
            justify="left",
            anchor="w",
            fg="#1f2933",
            bg="#f0f0f0",
            font=("Segoe UI", 8, "bold"),
            wraplength=610,
        )
        self.fw_command_status_label.grid(row=8, column=0, columnspan=3, padx=8, pady=(0, 2), sticky="ew")
        ttk.Label(
            self.xyz_frame,
            textvariable=self.fw_command_echo_var,
            justify="left",
            wraplength=610,
            font=("Segoe UI", 8),
        ).grid(row=9, column=0, columnspan=3, padx=8, pady=(0, 10), sticky="w")

        self.pwm_frame = ttk.LabelFrame(self.command_frame, text="Manual PWM Debug")
        self.pwm_frame.grid(row=0, column=1, sticky="nsew", padx=6)
        self.pwm_frame.columnconfigure(1, weight=1)

        self._add_scale(self.pwm_frame, "Clamp", self.clamp_var, CLAMP_OPEN_PULSE, CLAMP_CLOSE_PULSE, 0)
        self._add_scale(self.pwm_frame, "Fore (Front/Back)", self.fore_aft_var, FORE_AFT_MIN_PULSE, FORE_AFT_MAX_PULSE, 1)
        self._add_scale(self.pwm_frame, "Gripper (Up/Down)", self.gripper_lift_var, GRIPPER_LIFT_MIN_PULSE, GRIPPER_LIFT_MAX_PULSE, 2)
        self._add_scale_with_directions(self.pwm_frame, "Right/Left", self.left_right_var, LEFT_RIGHT_MIN_PULSE, LEFT_RIGHT_MAX_PULSE, 3, "Right", "Left")
        ttk.Button(self.pwm_frame, text="Load Midpoint", command=self.load_startup_pwm).grid(row=4, column=0, padx=8, pady=10, sticky="ew")
        ttk.Button(self.pwm_frame, text="Send PWM", command=self.send_pwm_command).grid(row=4, column=1, padx=8, pady=10, sticky="ew")

        utility_frame = ttk.Frame(self.pwm_frame)
        utility_frame.grid(row=5, column=0, columnspan=2, sticky="ew", padx=8, pady=(0, 10))
        utility_frame.columnconfigure(0, weight=1)
        utility_frame.columnconfigure(1, weight=1)
        ttk.Button(utility_frame, text="Clamp Open", command=self.set_clamp_open).grid(row=0, column=0, padx=(0, 4), sticky="ew")
        ttk.Button(utility_frame, text="Clamp Close", command=self.set_clamp_close).grid(row=0, column=1, padx=(4, 0), sticky="ew")

        self.preview_frame = ttk.LabelFrame(self.command_frame, text="Motion Preview")
        self.preview_frame.grid(row=0, column=2, sticky="nsew", padx=(6, 0))
        self.preview_frame.columnconfigure(0, weight=1)

        self.preview_canvas = tk.Canvas(
            self.preview_frame,
            width=PREVIEW_CANVAS_WIDTH,
            height=PREVIEW_CANVAS_HEIGHT,
            bg="#07101b",
            highlightthickness=0,
        )
        self.preview_canvas.grid(row=0, column=0, sticky="nsew", padx=8, pady=(8, 4))
        self.preview_canvas.bind("<Configure>", self._handle_preview_canvas_configure)
        self.preview_canvas.bind("<Motion>", self._handle_preview_pointer_motion)
        self.preview_canvas.bind("<Leave>", self._handle_preview_pointer_leave)
        self.preview_canvas.bind("<ButtonPress-1>", self._handle_preview_left_press)
        self.preview_canvas.bind("<B1-Motion>", self._handle_preview_left_drag)
        self.preview_canvas.bind("<ButtonRelease-1>", self._handle_preview_left_release)
        self.preview_canvas.bind("<ButtonPress-3>", self._handle_preview_right_press)
        self.preview_canvas.bind("<B3-Motion>", self._handle_preview_right_drag)
        self.preview_canvas.bind("<ButtonRelease-3>", self._handle_preview_right_release)
        workspace_controls_frame = ttk.Frame(self.preview_frame)
        workspace_controls_frame.grid(row=1, column=0, sticky="ew", padx=8, pady=(0, 4))
        workspace_controls_frame.columnconfigure(1, weight=1)
        ttk.Label(workspace_controls_frame, text="3D Mode", font=("Segoe UI", 9, "bold")).grid(row=0, column=0, padx=(0, 6), pady=0, sticky="w")
        self.workspace_mode_combo = ttk.Combobox(
            workspace_controls_frame,
            textvariable=self.workspace_interaction_mode_var,
            state="readonly",
            values=("Hold Preview / Release Send", "Live Follow"),
            width=28,
        )
        self.workspace_mode_combo.grid(row=0, column=1, padx=(0, 8), pady=0, sticky="ew")
        ttk.Label(workspace_controls_frame, text="Reaction (ms)", font=("Segoe UI", 9, "bold")).grid(row=0, column=2, padx=(0, 6), pady=0, sticky="w")
        self.workspace_reaction_time_spinbox = ttk.Spinbox(
            workspace_controls_frame,
            from_=WORKSPACE_REACTION_TIME_MIN_MS,
            to=WORKSPACE_REACTION_TIME_MAX_MS,
            increment=20,
            textvariable=self.workspace_reaction_time_var,
            width=8,
        )
        self.workspace_reaction_time_spinbox.grid(row=0, column=3, pady=0, sticky="w")
        ttk.Label(workspace_controls_frame, text="TOF Poll (ms)", font=("Segoe UI", 9, "bold")).grid(row=1, column=0, padx=(0, 6), pady=(6, 0), sticky="w")
        self.status_poll_interval_spinbox = ttk.Spinbox(
            workspace_controls_frame,
            from_=TOF_STATUS_POLL_INTERVAL_MIN_MS,
            to=TOF_STATUS_POLL_INTERVAL_MAX_MS,
            increment=50,
            textvariable=self.status_poll_interval_var,
            width=8,
        )
        self.status_poll_interval_spinbox.grid(row=1, column=1, pady=(6, 0), sticky="w")
        ttk.Label(
            workspace_controls_frame,
            text="FW TOF loop is 100 ms, so lower values will not refresh faster.",
            font=("Segoe UI", 8),
            justify="left",
        ).grid(row=1, column=2, columnspan=2, padx=(0, 6), pady=(6, 0), sticky="w")
        ttk.Label(self.preview_frame, textvariable=self.preview_legend_var, wraplength=492, justify="left", font=("Segoe UI", 9)).grid(
            row=2, column=0, sticky="ew", padx=8, pady=(0, 2)
        )
        ttk.Label(self.preview_frame, textvariable=self.preview_mode_var, wraplength=492, justify="left", font=("Segoe UI", 9)).grid(
            row=3, column=0, sticky="ew", padx=8, pady=(0, 2)
        )
        ttk.Label(self.preview_frame, textvariable=self.pose_summary_var, wraplength=492, justify="left", font=("Segoe UI", 9)).grid(
            row=4, column=0, sticky="ew", padx=8, pady=(0, 2)
        )
        ttk.Label(self.preview_frame, textvariable=self.target_summary_var, wraplength=492, justify="left", font=("Segoe UI", 9)).grid(
            row=5, column=0, sticky="ew", padx=8, pady=(0, 8)
        )
        ttk.Separator(self.preview_frame, orient="horizontal").grid(row=6, column=0, sticky="ew", padx=8, pady=(0, 6))
        ttk.Label(self.preview_frame, textvariable=self.firmware_motion_var, wraplength=492, justify="left", font=("Segoe UI", 9, "bold")).grid(
            row=7, column=0, sticky="ew", padx=8, pady=(0, 2)
        )
        ttk.Label(self.preview_frame, textvariable=self.firmware_pose_var, wraplength=492, justify="left", font=("Segoe UI", 9)).grid(
            row=8, column=0, sticky="ew", padx=8, pady=(0, 2)
        )
        ttk.Label(self.preview_frame, textvariable=self.firmware_target_var, wraplength=492, justify="left", font=("Segoe UI", 9)).grid(
            row=9, column=0, sticky="ew", padx=8, pady=(0, 8)
        )
        self.firmware_obstacle_label = tk.Label(
            self.preview_frame,
            textvariable=self.firmware_obstacle_var,
            justify="left",
            anchor="w",
            fg="#8a1020",
            bg="#f0f0f0",
            font=("Segoe UI", 9, "bold"),
            wraplength=492,
        )
        self.firmware_obstacle_label.grid(row=10, column=0, sticky="ew", padx=8, pady=(0, 8))

        self._apply_command_layout("wide")

        log_frame = ttk.LabelFrame(self.scrollable_frame, text="Serial Log")
        log_frame.grid(row=2, column=0, sticky="nsew", padx=12, pady=(0, 12))
        log_frame.columnconfigure(0, weight=1)
        log_frame.rowconfigure(0, weight=1)

        self.log_text = tk.Text(
            log_frame,
            wrap="word",
            state="disabled",
            bg="#09111d",
            fg="#dcecff",
            insertbackground="#dcecff",
            relief="flat",
        )
        self.log_text.grid(row=0, column=0, sticky="nsew")
        scrollbar = ttk.Scrollbar(log_frame, orient="vertical", command=self.log_text.yview)
        scrollbar.grid(row=0, column=1, sticky="ns")
        self.log_text.configure(yscrollcommand=scrollbar.set)

        bottom_frame = ttk.Frame(self.scrollable_frame)
        bottom_frame.grid(row=3, column=0, sticky="ew", padx=12, pady=(0, 12))
        ttk.Button(bottom_frame, text="HELP", command=lambda: self.send_command("HELP")).pack(side="left")
        ttk.Button(bottom_frame, textvariable=self.log_pause_button_var, command=self.toggle_log_pause).pack(side="left", padx=(8, 0))
        ttk.Button(bottom_frame, text="Copy Log", command=self.copy_log).pack(side="left", padx=8)
        ttk.Button(bottom_frame, text="Clear Log", command=self.clear_log).pack(side="left")

    def _handle_scrollable_frame_configure(self, _event) -> None:
        self._update_scroll_region()

    def _handle_scroll_canvas_configure(self, _event) -> None:
        self._update_scroll_region()

    def _handle_command_frame_configure(self, event) -> None:
        frame_width = event.width
        if frame_width < 980:
            layout_mode = "narrow"
        elif frame_width < 1480:
            layout_mode = "medium"
        else:
            layout_mode = "wide"

        self._apply_command_layout(layout_mode)

    def _handle_preview_canvas_configure(self, event) -> None:
        current_size = (event.width, event.height)
        if current_size == self._last_preview_canvas_size:
            return

        self._last_preview_canvas_size = current_size
        if event.width <= 1 or event.height <= 1:
            return

        self.after_idle(self._update_preview)

    def _workspace_is_live_follow_mode(self) -> bool:
        return self.workspace_interaction_mode_var.get() == "Live Follow"

    def _workspace_reaction_time_ms(self) -> int:
        try:
            reaction_time_ms = int(self.workspace_reaction_time_var.get().strip())
        except ValueError:
            reaction_time_ms = WORKSPACE_REACTION_TIME_DEFAULT_MS

        return max(WORKSPACE_REACTION_TIME_MIN_MS, min(WORKSPACE_REACTION_TIME_MAX_MS, reaction_time_ms))

    def _cancel_workspace_live_follow_send(self) -> None:
        if self._workspace_live_follow_after_id is not None:
            self.after_cancel(self._workspace_live_follow_after_id)
            self._workspace_live_follow_after_id = None

        self._workspace_live_follow_pending_target = None

    def _flush_workspace_live_follow_send(self) -> None:
        self._workspace_live_follow_after_id = None
        target = self._workspace_live_follow_pending_target
        self._workspace_live_follow_pending_target = None
        if target is None:
            return

        self._workspace_last_live_send_monotonic = time.monotonic()
        self._select_workspace_target(target, auto_send=self.serial_port is not None)

    def _queue_workspace_live_follow_target(self, target: tuple[int, int, int]) -> None:
        reaction_time_ms = self._workspace_reaction_time_ms()
        self._preview_workspace_target(
            target,
            status_text=(
                f"3D workspace live-follow x={target[0]} y={target[1]} z={target[2]} mm | reaction {reaction_time_ms} ms"
                if self.serial_port is not None
                else f"3D workspace live-follow preview x={target[0]} y={target[1]} z={target[2]} mm | connect serial to move"
            ),
        )

        if self.serial_port is None:
            self._cancel_workspace_live_follow_send()
            return

        now = time.monotonic()
        elapsed_ms = None
        if self._workspace_last_live_send_monotonic is not None:
            elapsed_ms = (now - self._workspace_last_live_send_monotonic) * 1000.0

        if reaction_time_ms == 0 or elapsed_ms is None or elapsed_ms >= reaction_time_ms:
            self._cancel_workspace_live_follow_send()
            self._workspace_last_live_send_monotonic = now
            self._select_workspace_target(target, auto_send=True)
            return

        self._workspace_live_follow_pending_target = target
        if self._workspace_live_follow_after_id is None:
            remaining_ms = max(1, int(round(reaction_time_ms - elapsed_ms)))
            self._workspace_live_follow_after_id = self.after(remaining_ms, self._flush_workspace_live_follow_send)

    def _handle_workspace_interaction_mode_change(self, *_args) -> None:
        self._workspace_drag_active = False
        self._workspace_drag_target = None
        self._cancel_workspace_preview_animation(clear_override=True)
        self._cancel_workspace_live_follow_send()
        self._update_workspace_interaction_controls()
        self._update_preview()

    def _update_workspace_interaction_controls(self) -> None:
        if self.workspace_reaction_time_spinbox is not None:
            self.workspace_reaction_time_spinbox.configure(state="normal" if self._workspace_is_live_follow_mode() else "disabled")

    def _is_pointer_in_workspace_panel(self, pointer_x: float, pointer_y: float) -> bool:
        if self._workspace_panel_bounds is None:
            return False

        left, top, right, bottom = self._workspace_panel_bounds
        return left <= pointer_x <= right and top <= pointer_y <= bottom

    def _pick_workspace_target(self, pointer_x: float, pointer_y: float, max_distance_px: float | None) -> tuple[int, int, int] | None:
        if not self._workspace_click_candidates:
            return None

        closest_target = None
        closest_distance_sq = float("inf")
        for screen_x, screen_y, target in self._workspace_click_candidates:
            distance_sq = ((screen_x - pointer_x) * (screen_x - pointer_x)) + ((screen_y - pointer_y) * (screen_y - pointer_y))
            if distance_sq < closest_distance_sq:
                closest_distance_sq = distance_sq
                closest_target = target

        if closest_target is None:
            return None
        if max_distance_px is not None and closest_distance_sq > (max_distance_px * max_distance_px):
            return None
        return closest_target

    def _handle_preview_pointer_motion(self, event) -> None:
        if self._workspace_drag_active or self._workspace_rotate_active:
            return

        hover_target = None
        if self._is_pointer_in_workspace_panel(event.x, event.y):
            hover_target = self._pick_workspace_target(event.x, event.y, max_distance_px=WORKSPACE_CLICK_PICK_RADIUS_PX)

        if hover_target == self._workspace_hover_target:
            return

        self._workspace_hover_target = hover_target
        if self._workspace_is_live_follow_mode() and hover_target is not None:
            self._queue_workspace_live_follow_target(hover_target)
            return

        self._update_preview()

    def _handle_preview_pointer_leave(self, _event) -> None:
        if self._workspace_drag_active or self._workspace_rotate_active:
            return
        if self._workspace_hover_target is None:
            return

        if self._workspace_is_live_follow_mode():
            self._cancel_workspace_live_follow_send()

        self._workspace_hover_target = None
        self._update_preview()

    def _handle_preview_left_press(self, event) -> None:
        if self._workspace_is_live_follow_mode():
            return
        if not self._is_pointer_in_workspace_panel(event.x, event.y):
            return

        closest_target = self._pick_workspace_target(event.x, event.y, max_distance_px=WORKSPACE_CLICK_PICK_RADIUS_PX)
        if closest_target is None:
            self.preview_mode_var.set("3D workspace press missed the reachable cloud. Hold on a cyan point to preview a move.")
            return

        self._workspace_drag_active = True
        self._workspace_drag_target = closest_target
        self._workspace_hover_target = closest_target
        self._preview_workspace_target(closest_target)

    def _handle_preview_left_drag(self, event) -> None:
        if self._workspace_is_live_follow_mode():
            return
        if not self._workspace_drag_active:
            return
        if not self._is_pointer_in_workspace_panel(event.x, event.y):
            return

        closest_target = self._pick_workspace_target(event.x, event.y, max_distance_px=None)
        if closest_target is None:
            return
        if closest_target == self._workspace_drag_target:
            return

        self._workspace_drag_target = closest_target
        self._workspace_hover_target = closest_target
        self._preview_workspace_target(closest_target)

    def _handle_preview_left_release(self, event) -> None:
        if self._workspace_is_live_follow_mode():
            return
        if not self._workspace_drag_active:
            return

        if self._is_pointer_in_workspace_panel(event.x, event.y):
            closest_target = self._pick_workspace_target(event.x, event.y, max_distance_px=None)
            if closest_target is not None and closest_target != self._workspace_drag_target:
                self._workspace_drag_target = closest_target
                self._workspace_hover_target = closest_target
                self._preview_workspace_target(closest_target)

        selected_target = self._workspace_drag_target
        self._workspace_drag_active = False
        self._workspace_drag_target = None
        if selected_target is None:
            self._cancel_workspace_preview_animation(clear_override=True)
            self._update_preview()
            return

        self._select_workspace_target(selected_target, auto_send=self.serial_port is not None)

    def _handle_preview_right_press(self, event) -> None:
        if not self._is_pointer_in_workspace_panel(event.x, event.y):
            return

        self._workspace_rotate_active = True
        self._workspace_rotate_last_pointer = (event.x, event.y)
        self._workspace_hover_target = None

    def _handle_preview_right_drag(self, event) -> None:
        if not self._workspace_rotate_active or self._workspace_rotate_last_pointer is None:
            return

        last_x, last_y = self._workspace_rotate_last_pointer
        delta_x = event.x - last_x
        delta_y = event.y - last_y
        if delta_x == 0 and delta_y == 0:
            return

        self._workspace_rotate_last_pointer = (event.x, event.y)
        self._workspace_view_azimuth_deg += delta_x * 0.45
        self._workspace_view_elevation = max(
            WORKSPACE_VIEW_MIN_ELEVATION,
            min(WORKSPACE_VIEW_MAX_ELEVATION, self._workspace_view_elevation - (delta_y * 0.004)),
        )
        self.preview_mode_var.set(
            f"3D workspace orbit azimuth={self._workspace_view_azimuth_deg:.0f} deg tilt={self._workspace_view_elevation:.2f}"
        )
        self._update_preview()

    def _handle_preview_right_release(self, _event) -> None:
        self._workspace_rotate_active = False
        self._workspace_rotate_last_pointer = None

    def _apply_command_layout(self, layout_mode: str) -> None:
        if self.command_frame is None or self.xyz_frame is None or self.pwm_frame is None or self.preview_frame is None:
            return
        if layout_mode == self.command_layout_mode:
            return

        for column_index in range(3):
            self.command_frame.columnconfigure(column_index, weight=0)
        for row_index in range(3):
            self.command_frame.rowconfigure(row_index, weight=0)

        if layout_mode == "wide":
            for column_index in range(3):
                self.command_frame.columnconfigure(column_index, weight=1)
            self.xyz_frame.grid_configure(row=0, column=0, columnspan=1, padx=(0, 6), pady=(0, 0))
            self.pwm_frame.grid_configure(row=0, column=1, columnspan=1, padx=6, pady=(0, 0))
            self.preview_frame.grid_configure(row=0, column=2, columnspan=1, padx=(6, 0), pady=(0, 0))
        elif layout_mode == "medium":
            self.command_frame.columnconfigure(0, weight=1)
            self.command_frame.columnconfigure(1, weight=1)
            self.command_frame.rowconfigure(1, weight=1)
            self.xyz_frame.grid_configure(row=0, column=0, columnspan=1, padx=(0, 6), pady=(0, 8))
            self.pwm_frame.grid_configure(row=0, column=1, columnspan=1, padx=(6, 0), pady=(0, 8))
            self.preview_frame.grid_configure(row=1, column=0, columnspan=2, padx=(0, 0), pady=(0, 0))
        else:
            self.command_frame.columnconfigure(0, weight=1)
            self.command_frame.rowconfigure(2, weight=1)
            self.xyz_frame.grid_configure(row=0, column=0, columnspan=1, padx=(0, 0), pady=(0, 8))
            self.pwm_frame.grid_configure(row=1, column=0, columnspan=1, padx=(0, 0), pady=(0, 8))
            self.preview_frame.grid_configure(row=2, column=0, columnspan=1, padx=(0, 0), pady=(0, 0))

        self.command_layout_mode = layout_mode
        self.after_idle(self._update_scroll_region)

    def _update_scroll_region(self) -> None:
        if self.scroll_canvas is None or self.scrollable_frame is None or self.scroll_window_id is None:
            return

        requested_width = self.scrollable_frame.winfo_reqwidth()
        requested_height = self.scrollable_frame.winfo_reqheight()
        canvas_width = self.scroll_canvas.winfo_width()
        canvas_height = self.scroll_canvas.winfo_height()

        window_width = requested_width if requested_width > canvas_width else canvas_width
        window_height = requested_height if requested_height > canvas_height else canvas_height

        self.scroll_canvas.itemconfigure(self.scroll_window_id, width=window_width, height=window_height)
        self.scroll_canvas.configure(scrollregion=(0, 0, window_width, window_height))

    def _install_variable_traces(self) -> None:
        variables = (
            self.clamp_var,
            self.fore_aft_var,
            self.gripper_lift_var,
            self.left_right_var,
            self.x_var,
            self.y_var,
            self.z_var,
        )
        for variable in variables:
            variable.trace_add("write", self._handle_visual_input_change)

    def _handle_visual_input_change(self, *_args) -> None:
        self._use_live_preview()
        self._update_xyz_entry_states()
        self._refresh_xyz_validation_state()

        if self._suspend_preview_updates:
            return

        self._workspace_drag_active = False
        self._workspace_drag_target = None
        self._cancel_workspace_preview_animation(clear_override=True)
        self._cancel_workspace_live_follow_send()
        self.preview_mode_var.set("Local preview follows manual PWM sliders.")
        self._update_preview()

    def _get_status_poll_interval_ms(self) -> int:
        raw_value = self.status_poll_interval_var.get().strip()
        try:
            interval_ms = int(raw_value)
        except ValueError:
            interval_ms = TOF_STATUS_POLL_INTERVAL_DEFAULT_MS

        interval_ms = max(TOF_STATUS_POLL_INTERVAL_MIN_MS, min(TOF_STATUS_POLL_INTERVAL_MAX_MS, interval_ms))
        normalized_value = str(interval_ms)
        if raw_value != normalized_value:
            self.status_poll_interval_var.set(normalized_value)
        return interval_ms

    def _handle_status_poll_interval_change(self, *_args) -> None:
        interval_ms = self._get_status_poll_interval_ms()
        if self.serial_port is not None:
            self._schedule_status_poll(initial_delay_ms=interval_ms)
        if self.preview_canvas is not None:
            self._update_preview()

    def _create_xyz_entry(self, parent, variable: tk.StringVar, row: int, column: int) -> tk.Entry:
        entry = tk.Entry(
            parent,
            textvariable=variable,
            relief="flat",
            bd=0,
            highlightthickness=1,
            bg="#ffffff",
            fg="#111111",
            insertbackground="#111111",
            font=("Segoe UI", 10),
        )
        entry.grid(row=row, column=column, padx=8, pady=4, sticky="ew")
        return entry

    def _guide_axis_bounds(self, axis_name: str) -> tuple[int, int]:
        minimum = math.ceil(self.workspace_bounds[f"min_{axis_name}"])
        maximum = math.floor(self.workspace_bounds[f"max_{axis_name}"])
        return minimum, maximum

    def _apply_xyz_entry_visual_state(self, entry: tk.Entry | None, is_valid: bool) -> None:
        if entry is None:
            return

        if is_valid:
            entry.configure(
                bg="#ffffff",
                fg="#111111",
                insertbackground="#111111",
                highlightbackground="#b8c2cc",
                highlightcolor="#2f6fed",
            )
        else:
            entry.configure(
                bg="#ffe7ea",
                fg="#8a1020",
                insertbackground="#8a1020",
                highlightbackground="#c83a4d",
                highlightcolor="#c83a4d",
            )

    def _axis_value_within_guide(self, axis_name: str, value_mm: int) -> bool:
        minimum, maximum = self._guide_axis_bounds(axis_name)
        return minimum <= value_mm <= maximum

    def _describe_target_reachability_issues(self, target: tuple[int, int, int]) -> list[str]:
        issues: list[str] = []
        radial_mm = math.hypot(target[0], target[1])
        minimum_radius = math.ceil(self.workspace_bounds["min_radius"])
        maximum_radius = math.floor(self.workspace_bounds["max_radius"])

        if not minimum_radius <= radial_mm <= maximum_radius:
            issues.append(f"Reach radius guide is {minimum_radius}..{maximum_radius} mm, got {radial_mm:.0f} mm")

        if self._solve_safe_ik(target[0], target[1], target[2]) is None and not issues:
            issues.append("Local IK cannot solve this XYZ target safely")

        return issues

    def _update_xyz_entry_states(self) -> None:
        axis_inputs = (
            ("x", self.x_var, self.x_entry),
            ("y", self.y_var, self.y_entry),
            ("z", self.z_var, self.z_entry),
        )
        parsed_values: dict[str, int] = {}
        has_invalid_axis = False

        for axis_name, variable, entry in axis_inputs:
            try:
                value_mm = int(variable.get().strip())
            except ValueError:
                self._apply_xyz_entry_visual_state(entry, False)
                has_invalid_axis = True
                continue

            parsed_values[axis_name] = value_mm
            is_valid = self._axis_value_within_guide(axis_name, value_mm)
            self._apply_xyz_entry_visual_state(entry, is_valid)
            if not is_valid:
                has_invalid_axis = True

        if has_invalid_axis or len(parsed_values) != 3:
            return

        target = (parsed_values["x"], parsed_values["y"], parsed_values["z"])
        if self._describe_target_reachability_issues(target):
            for _axis_name, _variable, entry in axis_inputs:
                self._apply_xyz_entry_visual_state(entry, False)

    def _describe_target_guide_violations(self, target: tuple[int, int, int]) -> list[str]:
        violations: list[str] = []

        for axis_name, value_mm in zip(("x", "y", "z"), target):
            minimum, maximum = self._guide_axis_bounds(axis_name)
            if not minimum <= value_mm <= maximum:
                violations.append(f"{axis_name.upper()} must stay within {minimum}..{maximum} mm, got {value_mm} mm")

        if not violations:
            violations.extend(self._describe_target_reachability_issues(target))

        return violations

    def _describe_current_xyz_input_issues(self) -> list[str]:
        issues: list[str] = []
        axis_inputs = (
            ("x", self.x_var),
            ("y", self.y_var),
            ("z", self.z_var),
        )
        parsed_values: dict[str, int] = {}

        for axis_name, variable in axis_inputs:
            raw_value = variable.get().strip()
            try:
                value_mm = int(raw_value)
            except ValueError:
                issues.append(f"{axis_name.upper()} must be an integer in millimeters")
                continue

            parsed_values[axis_name] = value_mm
            minimum, maximum = self._guide_axis_bounds(axis_name)
            if not minimum <= value_mm <= maximum:
                issues.append(f"{axis_name.upper()} guide is {minimum}..{maximum} mm, got {value_mm} mm")

        if not issues and len(parsed_values) == 3:
            issues.extend(self._describe_target_reachability_issues((parsed_values["x"], parsed_values["y"], parsed_values["z"])))

        return issues

    def _refresh_xyz_validation_state(self) -> None:
        issues = self._describe_current_xyz_input_issues()

        if issues:
            self.xyz_validation_var.set("Blocked: " + " | ".join(issues))
            if self.send_xyz_button is not None:
                self.send_xyz_button.configure(state="disabled")
            return

        self.xyz_validation_var.set("Ready: XYZ is inside the local guide range.")
        if self.send_xyz_button is not None:
            self.send_xyz_button.configure(state="normal")

    def _set_fw_command_feedback(self, status_text: str, echo_text: str, tone: str = "neutral") -> None:
        tone_colors = {
            "neutral": "#1f2933",
            "waiting": "#9a6700",
            "success": "#0f6b3c",
            "error": "#b42318",
        }

        self.fw_command_status_var.set(status_text)
        self.fw_command_echo_var.set(echo_text)
        if self.fw_command_status_label is not None:
            self.fw_command_status_label.configure(fg=tone_colors.get(tone, tone_colors["neutral"]))

    def _set_firmware_obstacle_feedback(self, text: str, tone: str) -> None:
        tone_colors = {
            "neutral": "#1f2933",
            "success": "#0f6b3c",
            "warning": "#9a6700",
            "error": "#b42318",
        }

        self.firmware_obstacle_var.set(text)
        if self.firmware_obstacle_label is not None:
            self.firmware_obstacle_label.configure(fg=tone_colors.get(tone, tone_colors["neutral"]))

    def _set_obstacle_monitor_state(
        self,
        state: str,
        range_mm: int,
        threshold_mm: int,
        clear_mm: int,
        device_name: str,
    ) -> None:
        self._obstacle_monitor_state = state
        self._obstacle_range_mm = range_mm
        self._obstacle_threshold_mm = threshold_mm
        self._obstacle_clear_mm = clear_mm
        self._obstacle_device_name = device_name
        if self.preview_canvas is not None:
            self._update_preview()

    def _set_imu_monitor_state(
        self,
        state: str,
        pitch_deg: int,
        roll_deg: int,
        gyro_mag_mdps: int,
        temp_dc: int,
        device_name: str,
    ) -> None:
        self._imu_monitor_state = state
        self._imu_pitch_deg = pitch_deg
        self._imu_roll_deg = roll_deg
        self._imu_gyro_mag_mdps = gyro_mag_mdps
        self._imu_temp_dc = temp_dc
        self._imu_device_name = device_name
        if self.preview_canvas is not None:
            self._update_preview()

    def _add_scale(self, parent, label, variable, minimum, maximum, row) -> None:
        value_label = ttk.Label(parent, textvariable=variable, width=6)
        ttk.Label(parent, text=label).grid(row=row, column=0, padx=8, pady=(10, 0), sticky="w")
        scale = tk.Scale(
            parent,
            from_=minimum,
            to=maximum,
            orient="horizontal",
            resolution=1,
            variable=variable,
            length=340,
            bg="#f3f7fb",
            troughcolor="#bfd9ff",
            activebackground="#ff9447",
            highlightthickness=0,
        )
        scale.grid(row=row, column=1, padx=8, pady=(10, 0), sticky="ew")
        value_label.grid(row=row, column=2, padx=(0, 8), pady=(10, 0), sticky="w")

    def _add_scale_with_directions(self, parent, label, variable, minimum, maximum, row, left_label, right_label) -> None:
        value_label = ttk.Label(parent, textvariable=variable, width=6)
        ttk.Label(parent, text=label).grid(row=row, column=0, padx=8, pady=(10, 0), sticky="w")

        # 方向標籤框架
        direction_frame = ttk.Frame(parent)
        direction_frame.grid(row=row+1, column=1, padx=8, pady=(0, 10), sticky="ew")
        direction_frame.columnconfigure(1, weight=1)

        ttk.Label(direction_frame, text=left_label, foreground="#666666").grid(row=0, column=0, sticky="w")
        ttk.Label(direction_frame, text=right_label, foreground="#666666").grid(row=0, column=2, sticky="e")

        scale = tk.Scale(
            parent,
            from_=minimum,
            to=maximum,
            orient="horizontal",
            resolution=1,
            variable=variable,
            length=340,
            bg="#f3f7fb",
            troughcolor="#bfd9ff",
            activebackground="#ff9447",
            highlightthickness=0,
        )
        scale.grid(row=row, column=1, padx=8, pady=(10, 0), sticky="ew")
        value_label.grid(row=row, column=2, padx=(0, 8), pady=(10, 0), sticky="w")

    def _degrees_to_radians(self, degrees: float) -> float:
        return math.radians(degrees)

    def _create_initial_preview_joint_angles(self) -> tuple[float, float, float]:
        return (
            self._degrees_to_radians(STARTUP_PREVIEW_YAW_DEG),
            self._degrees_to_radians(STARTUP_PREVIEW_SHOULDER_DEG),
            self._degrees_to_radians(STARTUP_PREVIEW_GRIPPER_DEG),
        )

    def _use_live_preview(self) -> None:
        self._initial_preview_joint_angles = None

    def _display_joint_angles(self) -> tuple[float, float, float]:
        if self._workspace_preview_joint_angles is not None:
            return self._workspace_preview_joint_angles
        if self._initial_preview_joint_angles is not None:
            return self._initial_preview_joint_angles
        return self._current_joint_angles()

    def _cancel_workspace_preview_animation(self, clear_override: bool) -> None:
        if self._workspace_preview_after_id is not None:
            self.after_cancel(self._workspace_preview_after_id)
            self._workspace_preview_after_id = None

        self._workspace_preview_target_angles = None
        if clear_override:
            self._workspace_preview_joint_angles = None

    def _animate_workspace_preview(self) -> None:
        self._workspace_preview_after_id = None
        if self._workspace_preview_joint_angles is None or self._workspace_preview_target_angles is None:
            return

        next_angles = []
        has_remaining_delta = False
        for current_angle, target_angle in zip(self._workspace_preview_joint_angles, self._workspace_preview_target_angles):
            delta = target_angle - current_angle
            if abs(delta) <= 0.002:
                next_angles.append(target_angle)
                continue

            next_angles.append(current_angle + (delta * WORKSPACE_PREVIEW_EASING))
            has_remaining_delta = True

        self._workspace_preview_joint_angles = tuple(next_angles)
        self._update_preview()
        if has_remaining_delta:
            self._workspace_preview_after_id = self.after(16, self._animate_workspace_preview)

    def _preview_workspace_target(self, target: tuple[int, int, int], status_text: str | None = None) -> None:
        solution = self._solve_safe_ik(target[0], target[1], target[2])
        if solution is None:
            self.preview_mode_var.set("3D workspace preview skipped because this sampled point is no longer solvable.")
            return

        self._use_live_preview()
        self._set_xyz_values(target[0], target[1], target[2])
        if self._workspace_preview_joint_angles is None:
            self._workspace_preview_joint_angles = self._display_joint_angles()
        self._workspace_preview_target_angles = solution
        self.preview_mode_var.set(
            status_text
            if status_text is not None
            else f"3D workspace preview x={target[0]} y={target[1]} z={target[2]} mm | hold to scrub, release to send"
        )
        if self._workspace_preview_after_id is None:
            self._animate_workspace_preview()

    def _map_value(self, value: float, source_minimum: float, source_maximum: float, target_minimum: float, target_maximum: float) -> float:
        if source_maximum <= source_minimum:
            return target_minimum

        ratio = (value - source_minimum) / (source_maximum - source_minimum)
        ratio = max(0.0, min(1.0, ratio))
        return target_minimum + (ratio * (target_maximum - target_minimum))

    def _current_joint_angles(self) -> tuple[float, float, float]:
        yaw_rad = self._map_value(
            self.left_right_var.get(),
            LEFT_RIGHT_MIN_PULSE,
            LEFT_RIGHT_MAX_PULSE,
            self._degrees_to_radians(LEFT_RIGHT_MIN_DEG),
            self._degrees_to_radians(LEFT_RIGHT_MAX_DEG),
        )
        shoulder_rad = self._map_value(
            self.fore_aft_var.get(),
            FORE_AFT_MIN_PULSE,
            FORE_AFT_MAX_PULSE,
            self._degrees_to_radians(FORE_AFT_MIN_DEG),
            self._degrees_to_radians(FORE_AFT_MAX_DEG),
        )
        gripper_rad = self._map_value(
            self.gripper_lift_var.get(),
            GRIPPER_LIFT_MIN_PULSE,
            GRIPPER_LIFT_MAX_PULSE,
            self._degrees_to_radians(GRIPPER_LIFT_MIN_DEG),
            self._degrees_to_radians(GRIPPER_LIFT_MAX_DEG),
        )
        return yaw_rad, shoulder_rad, gripper_rad

    def _forward_pose(self, yaw_rad: float, shoulder_rad: float, gripper_rad: float) -> dict[str, float]:
        tool_angle_rad = shoulder_rad + gripper_rad
        radial_mm = (ARM_SHOULDER_LENGTH_MM * math.cos(shoulder_rad)) + (ARM_DISTAL_LENGTH_MM * math.cos(tool_angle_rad))
        x_mm = radial_mm * math.cos(yaw_rad)
        y_mm = radial_mm * math.sin(yaw_rad)
        z_mm = ARM_BASE_HEIGHT_MM + (ARM_SHOULDER_LENGTH_MM * math.sin(shoulder_rad)) + (ARM_DISTAL_LENGTH_MM * math.sin(tool_angle_rad))
        elbow_x_mm = ARM_SHOULDER_LENGTH_MM * math.cos(shoulder_rad)
        elbow_z_mm = ARM_BASE_HEIGHT_MM + (ARM_SHOULDER_LENGTH_MM * math.sin(shoulder_rad))
        wrist_x_mm = elbow_x_mm + (ARM_DISTAL_LENGTH_MM * math.cos(tool_angle_rad))
        wrist_z_mm = elbow_z_mm + (ARM_DISTAL_LENGTH_MM * math.sin(tool_angle_rad))
        return {
            "x": x_mm,
            "y": y_mm,
            "z": z_mm,
            "radial": radial_mm,
            "tool_angle": tool_angle_rad,
            "elbow_x": elbow_x_mm,
            "elbow_z": elbow_z_mm,
            "wrist_x": wrist_x_mm,
            "wrist_z": wrist_z_mm,
        }

    def _safe_joint_limits(self) -> tuple[tuple[float, float], tuple[float, float], tuple[float, float]]:
        return (
            (self._degrees_to_radians(LEFT_RIGHT_MIN_DEG), self._degrees_to_radians(LEFT_RIGHT_MAX_DEG)),
            (self._degrees_to_radians(FORE_AFT_MIN_DEG), self._degrees_to_radians(FORE_AFT_MAX_DEG)),
            (self._degrees_to_radians(GRIPPER_LIFT_MIN_DEG), self._degrees_to_radians(GRIPPER_LIFT_MAX_DEG)),
        )

    def _estimate_workspace_bounds(self) -> dict[str, float]:
        yaw_limits, shoulder_limits, gripper_limits = self._safe_joint_limits()
        has_sample = False
        bounds = {
            "min_x": 0.0,
            "max_x": 0.0,
            "min_y": 0.0,
            "max_y": 0.0,
            "min_z": 0.0,
            "max_z": 0.0,
            "min_radius": 0.0,
            "max_radius": 0.0,
        }

        def sample_value(minimum: float, maximum: float, index: int) -> float:
            if SAFE_WORKSPACE_SAMPLES_PER_JOINT <= 1:
                return minimum
            return minimum + ((maximum - minimum) * index / (SAFE_WORKSPACE_SAMPLES_PER_JOINT - 1))

        for yaw_index in range(SAFE_WORKSPACE_SAMPLES_PER_JOINT):
            yaw_rad = sample_value(yaw_limits[0], yaw_limits[1], yaw_index)
            for shoulder_index in range(SAFE_WORKSPACE_SAMPLES_PER_JOINT):
                shoulder_rad = sample_value(shoulder_limits[0], shoulder_limits[1], shoulder_index)
                for gripper_index in range(SAFE_WORKSPACE_SAMPLES_PER_JOINT):
                    gripper_rad = sample_value(gripper_limits[0], gripper_limits[1], gripper_index)
                    pose = self._forward_pose(yaw_rad, shoulder_rad, gripper_rad)
                    if pose["z"] < SAFE_WORKSPACE_MIN_Z_MM:
                        continue

                    horizontal_radius_mm = math.hypot(pose["x"], pose["y"])
                    if not has_sample:
                        bounds["min_x"] = pose["x"]
                        bounds["max_x"] = pose["x"]
                        bounds["min_y"] = pose["y"]
                        bounds["max_y"] = pose["y"]
                        bounds["min_z"] = pose["z"]
                        bounds["max_z"] = pose["z"]
                        bounds["min_radius"] = horizontal_radius_mm
                        bounds["max_radius"] = horizontal_radius_mm
                        has_sample = True
                        continue

                    bounds["min_x"] = min(bounds["min_x"], pose["x"])
                    bounds["max_x"] = max(bounds["max_x"], pose["x"])
                    bounds["min_y"] = min(bounds["min_y"], pose["y"])
                    bounds["max_y"] = max(bounds["max_y"], pose["y"])
                    bounds["min_z"] = min(bounds["min_z"], pose["z"])
                    bounds["max_z"] = max(bounds["max_z"], pose["z"])
                    bounds["min_radius"] = min(bounds["min_radius"], horizontal_radius_mm)
                    bounds["max_radius"] = max(bounds["max_radius"], horizontal_radius_mm)

        if not has_sample:
            bounds["min_z"] = SAFE_WORKSPACE_MIN_Z_MM

        return bounds

    def _sample_workspace_preview_points(self) -> list[tuple[int, int, int]]:
        yaw_limits, shoulder_limits, gripper_limits = self._safe_joint_limits()
        sampled_points: dict[tuple[int, int, int], tuple[int, int, int]] = {}

        def sample_value(minimum: float, maximum: float, index: int) -> float:
            if WORKSPACE_PREVIEW_SAMPLES_PER_JOINT <= 1:
                return minimum
            return minimum + ((maximum - minimum) * index / (WORKSPACE_PREVIEW_SAMPLES_PER_JOINT - 1))

        for yaw_index in range(WORKSPACE_PREVIEW_SAMPLES_PER_JOINT):
            yaw_rad = sample_value(yaw_limits[0], yaw_limits[1], yaw_index)
            for shoulder_index in range(WORKSPACE_PREVIEW_SAMPLES_PER_JOINT):
                shoulder_rad = sample_value(shoulder_limits[0], shoulder_limits[1], shoulder_index)
                for gripper_index in range(WORKSPACE_PREVIEW_SAMPLES_PER_JOINT):
                    gripper_rad = sample_value(gripper_limits[0], gripper_limits[1], gripper_index)
                    pose = self._forward_pose(yaw_rad, shoulder_rad, gripper_rad)
                    if pose["z"] < SAFE_WORKSPACE_MIN_Z_MM:
                        continue

                    rounded_point = (
                        int(round(pose["x"])),
                        int(round(pose["y"])),
                        int(round(pose["z"])),
                    )
                    if self._solve_safe_ik(*rounded_point) is None:
                        continue
                    quantized_key = (
                        int(round(rounded_point[0] / WORKSPACE_PREVIEW_QUANTIZATION_MM)),
                        int(round(rounded_point[1] / WORKSPACE_PREVIEW_QUANTIZATION_MM)),
                        int(round(rounded_point[2] / WORKSPACE_PREVIEW_QUANTIZATION_MM)),
                    )
                    sampled_points[quantized_key] = rounded_point

        points = list(sampled_points.values())
        points.sort(key=lambda point: (point[2], point[0] + point[1], point[0]))
        return points

    def _project_workspace_point(self, x_mm: float, y_mm: float, z_mm: float) -> tuple[float, float]:
        azimuth_rad = self._degrees_to_radians(self._workspace_view_azimuth_deg)
        projected_x = (x_mm * math.sin(azimuth_rad)) - (y_mm * math.cos(azimuth_rad))
        forward_depth = (x_mm * math.cos(azimuth_rad)) + (y_mm * math.sin(azimuth_rad))
        projected_y = z_mm - (forward_depth * self._workspace_view_elevation)
        return projected_x, projected_y

    def _estimate_workspace_projection_bounds(self, points: list[tuple[int, int, int]]) -> dict[str, float]:
        projection_bounds = {
            "min_x": 0.0,
            "max_x": 0.0,
            "min_y": 0.0,
            "max_y": 0.0,
        }
        has_projection = False

        anchor_points = list(points)
        anchor_points.extend(
            [
                (0, 0, 0),
                (int(round(self.workspace_bounds["max_radius"] * WORKSPACE_AXIS_GUIDE_RATIO)), 0, 0),
                (0, int(round(self.workspace_bounds["max_radius"] * WORKSPACE_AXIS_GUIDE_RATIO)), 0),
                (0, 0, int(round(self.workspace_bounds["max_z"] * WORKSPACE_AXIS_GUIDE_RATIO))),
            ]
        )

        for x_mm, y_mm, z_mm in anchor_points:
            projected_x, projected_y = self._project_workspace_point(x_mm, y_mm, z_mm)
            if not has_projection:
                projection_bounds["min_x"] = projected_x
                projection_bounds["max_x"] = projected_x
                projection_bounds["min_y"] = projected_y
                projection_bounds["max_y"] = projected_y
                has_projection = True
                continue

            projection_bounds["min_x"] = min(projection_bounds["min_x"], projected_x)
            projection_bounds["max_x"] = max(projection_bounds["max_x"], projected_x)
            projection_bounds["min_y"] = min(projection_bounds["min_y"], projected_y)
            projection_bounds["max_y"] = max(projection_bounds["max_y"], projected_y)

        padding_x = max(8.0, (projection_bounds["max_x"] - projection_bounds["min_x"]) * WORKSPACE_PROJECTION_PADDING_RATIO)
        padding_y = max(8.0, (projection_bounds["max_y"] - projection_bounds["min_y"]) * WORKSPACE_PROJECTION_PADDING_RATIO)
        projection_bounds["min_x"] -= padding_x
        projection_bounds["max_x"] += padding_x
        projection_bounds["min_y"] -= padding_y
        projection_bounds["max_y"] += padding_y

        return projection_bounds

    def _format_axis_range_text(self, axis_name: str) -> str:
        minimum, maximum = self._guide_axis_bounds(axis_name)
        return f"guide {minimum}..{maximum}"

    def _guide_axis_midpoint(self, axis_name: str) -> int:
        minimum, maximum = self._guide_axis_bounds(axis_name)
        return int(round((minimum + maximum) * 0.5))

    def _format_workspace_note_text(self) -> str:
        return (
            "Conservative local workspace guide from safe joint limits: "
            f"radius {self.workspace_bounds['min_radius']:.0f}..{self.workspace_bounds['max_radius']:.0f} mm, "
            f"z >= {SAFE_WORKSPACE_MIN_Z_MM:.0f} mm."
        )

    def _estimate_preview_bounds(self) -> dict[str, float]:
        _, shoulder_limits, gripper_limits = self._safe_joint_limits()
        bounds = {
            "min_x": 0.0,
            "max_x": 0.0,
            "min_z": 0.0,
            "max_z": ARM_BASE_HEIGHT_MM,
        }

        for shoulder_index in range(SAFE_WORKSPACE_SAMPLES_PER_JOINT):
            shoulder_rad = shoulder_limits[0] + ((shoulder_limits[1] - shoulder_limits[0]) * shoulder_index / (SAFE_WORKSPACE_SAMPLES_PER_JOINT - 1))
            for gripper_index in range(SAFE_WORKSPACE_SAMPLES_PER_JOINT):
                gripper_rad = gripper_limits[0] + ((gripper_limits[1] - gripper_limits[0]) * gripper_index / (SAFE_WORKSPACE_SAMPLES_PER_JOINT - 1))
                pose = self._forward_pose(0.0, shoulder_rad, gripper_rad)
                bounds["min_x"] = min(bounds["min_x"], pose["elbow_x"], pose["wrist_x"])
                bounds["max_x"] = max(bounds["max_x"], pose["elbow_x"], pose["wrist_x"])
                bounds["min_z"] = min(bounds["min_z"], pose["elbow_z"], pose["wrist_z"])
                bounds["max_z"] = max(bounds["max_z"], pose["elbow_z"], pose["wrist_z"])

        horizontal_padding_mm = max(45.0, ARM_SHOULDER_LENGTH_MM * 0.45)
        bounds["min_x"] = min(bounds["min_x"] - 20.0, -horizontal_padding_mm)
        bounds["max_x"] += horizontal_padding_mm
        bounds["min_z"] -= 25.0
        bounds["max_z"] += 20.0
        return bounds

    def _angles_within_limits(self, yaw_rad: float, shoulder_rad: float, gripper_rad: float) -> bool:
        yaw_limits, shoulder_limits, gripper_limits = self._safe_joint_limits()
        return (
            yaw_limits[0] <= yaw_rad <= yaw_limits[1]
            and shoulder_limits[0] <= shoulder_rad <= shoulder_limits[1]
            and gripper_limits[0] <= gripper_rad <= gripper_limits[1]
        )

    def _solve_safe_ik(self, x_mm: int, y_mm: int, z_mm: int) -> tuple[float, float, float] | None:
        if z_mm < 0:
            return None

        radial_mm = math.hypot(x_mm, y_mm)
        z_from_shoulder_mm = z_mm - ARM_BASE_HEIGHT_MM
        denominator = 2.0 * ARM_SHOULDER_LENGTH_MM * ARM_DISTAL_LENGTH_MM
        if denominator == 0.0:
            return None

        cos_elbow = ((radial_mm * radial_mm) + (z_from_shoulder_mm * z_from_shoulder_mm) - (ARM_SHOULDER_LENGTH_MM * ARM_SHOULDER_LENGTH_MM) - (ARM_DISTAL_LENGTH_MM * ARM_DISTAL_LENGTH_MM)) / denominator
        if cos_elbow < -1.0 or cos_elbow > 1.0:
            return None

        yaw_rad = math.atan2(y_mm, x_mm)
        if not (self._degrees_to_radians(LEFT_RIGHT_MIN_DEG) <= yaw_rad <= self._degrees_to_radians(LEFT_RIGHT_MAX_DEG)):
            return None

        sin_elbow_magnitude = math.sqrt(max(0.0, 1.0 - (cos_elbow * cos_elbow)))
        for sin_elbow in (-sin_elbow_magnitude, sin_elbow_magnitude):
            gripper_rad = math.atan2(sin_elbow, cos_elbow)
            shoulder_rad = math.atan2(z_from_shoulder_mm, radial_mm) - math.atan2(
                ARM_DISTAL_LENGTH_MM * sin_elbow,
                ARM_SHOULDER_LENGTH_MM + (ARM_DISTAL_LENGTH_MM * cos_elbow),
            )
            if self._angles_within_limits(yaw_rad, shoulder_rad, gripper_rad):
                return yaw_rad, shoulder_rad, gripper_rad

        return None

    def _angles_to_pwm(self, yaw_rad: float, shoulder_rad: float, gripper_rad: float) -> tuple[int, int, int]:
        left_right_pulse = int(round(self._map_value(
            yaw_rad,
            self._degrees_to_radians(LEFT_RIGHT_MIN_DEG),
            self._degrees_to_radians(LEFT_RIGHT_MAX_DEG),
            LEFT_RIGHT_MIN_PULSE,
            LEFT_RIGHT_MAX_PULSE,
        )))
        fore_aft_pulse = int(round(self._map_value(
            shoulder_rad,
            self._degrees_to_radians(FORE_AFT_MIN_DEG),
            self._degrees_to_radians(FORE_AFT_MAX_DEG),
            FORE_AFT_MIN_PULSE,
            FORE_AFT_MAX_PULSE,
        )))
        gripper_lift_pulse = int(round(self._map_value(
            gripper_rad,
            self._degrees_to_radians(GRIPPER_LIFT_MIN_DEG),
            self._degrees_to_radians(GRIPPER_LIFT_MAX_DEG),
            GRIPPER_LIFT_MIN_PULSE,
            GRIPPER_LIFT_MAX_PULSE,
        )))
        return left_right_pulse, fore_aft_pulse, gripper_lift_pulse

    def _set_pwm_values(self, clamp_pulse: int, fore_aft_pulse: int, gripper_lift_pulse: int, left_right_pulse: int) -> None:
        self._suspend_preview_updates = True
        self.clamp_var.set(clamp_pulse)
        self.fore_aft_var.set(fore_aft_pulse)
        self.gripper_lift_var.set(gripper_lift_pulse)
        self.left_right_var.set(left_right_pulse)
        self._suspend_preview_updates = False
        self._update_xyz_entry_states()
        self._update_preview()

    def _current_target(self) -> tuple[int, int, int] | None:
        try:
            x_mm = int(self.x_var.get().strip())
            y_mm = int(self.y_var.get().strip())
            z_mm = int(self.z_var.get().strip())
        except ValueError:
            return None
        return x_mm, y_mm, z_mm

    def _set_xyz_values(self, x_mm: int, y_mm: int, z_mm: int) -> None:
        self._suspend_preview_updates = True
        self.x_var.set(str(x_mm))
        self.y_var.set(str(y_mm))
        self.z_var.set(str(z_mm))
        self._suspend_preview_updates = False
        self._update_xyz_entry_states()
        self._refresh_xyz_validation_state()

    def _select_workspace_target(self, target: tuple[int, int, int], auto_send: bool) -> None:
        solution = self._solve_safe_ik(target[0], target[1], target[2])
        if solution is None:
            self.preview_mode_var.set("3D workspace release hit a point that local IK can no longer solve safely.")
            self._cancel_workspace_preview_animation(clear_override=True)
            self._update_preview()
            return

        self._use_live_preview()
        self._cancel_workspace_preview_animation(clear_override=True)
        self._set_xyz_values(target[0], target[1], target[2])
        if auto_send:
            self.preview_mode_var.set(f"3D workspace selected x={target[0]} y={target[1]} z={target[2]} mm | sending XYZ")
            self.send_xyz_command()
            return

        left_right_pulse, fore_aft_pulse, gripper_lift_pulse = self._angles_to_pwm(*solution)
        self._set_pwm_values(self.clamp_var.get(), fore_aft_pulse, gripper_lift_pulse, left_right_pulse)
        self.preview_mode_var.set(
            f"3D workspace selected x={target[0]} y={target[1]} z={target[2]} mm | connect serial or press Send XYZ to move"
        )

    def _update_preview(self) -> None:
        yaw_rad, shoulder_rad, gripper_rad = self._display_joint_angles()
        pose = self._forward_pose(yaw_rad, shoulder_rad, gripper_rad)
        target = self._current_target()

        self.pose_summary_var.set(
            f"Local pose x={pose['x']:.0f} y={pose['y']:.0f} z={pose['z']:.0f} mm | "
            f"yaw={math.degrees(yaw_rad):.0f} fore={math.degrees(shoulder_rad):.0f} gripper_lift={math.degrees(gripper_rad):.0f} deg"
        )
        if target is None:
            self.target_summary_var.set("Local target needs valid integer XYZ values to preview the marker.")
        else:
            self.target_summary_var.set(f"Local target x={target[0]} y={target[1]} z={target[2]} mm")

        self._draw_preview(pose, target, yaw_rad)

    def _draw_preview(self, pose: dict[str, float], target: tuple[int, int, int] | None, yaw_rad: float) -> None:
        canvas = self.preview_canvas
        canvas.delete("all")
        self._workspace_click_candidates = []
        self._workspace_panel_bounds = None

        width = canvas.winfo_width()
        height = canvas.winfo_height()
        if width <= 1:
            width = PREVIEW_CANVAS_WIDTH
        if height <= 1:
            height = PREVIEW_CANVAS_HEIGHT

        content_left = max(0.0, (width - PREVIEW_CANVAS_WIDTH) * 0.5)
        content_top = max(0.0, (height - PREVIEW_CANVAS_HEIGHT) * 0.5)
        content_bottom = content_top + PREVIEW_CANVAS_HEIGHT

        canvas.create_rectangle(0, 0, width, height, fill="#07101b", outline="")
        canvas.create_oval(content_left - 80, content_top - 70, content_left + 320, content_top + 240, fill="#122b49", outline="")
        canvas.create_oval(content_left + 380, content_top + 40, content_left + 820, content_top + 360, fill="#1a203d", outline="")
        canvas.create_oval(content_left - 60, content_top + 320, content_left + 360, content_top + 620, fill="#0e1f37", outline="")
        canvas.create_rectangle(0, content_bottom - 52, width, content_bottom, fill="#081723", outline="")

        top_row_top = content_top + 16
        top_row_bottom = content_top + 250
        bottom_row_top = content_top + 270
        bottom_row_bottom = content_top + PREVIEW_CANVAS_HEIGHT - 14
        left_col_left = content_left + 14
        left_col_right = content_left + 372
        right_col_left = content_left + 388
        right_col_right = content_left + PREVIEW_CANVAS_WIDTH - 14

        side_left = left_col_left
        side_top = bottom_row_top
        side_right = left_col_right
        side_bottom = bottom_row_bottom
        canvas.create_rectangle(side_left, side_top, side_right, side_bottom, fill="#0c1b2a", outline="#1f4165", width=2)

        min_x_mm = self.preview_bounds["min_x"]
        max_x_mm = self.preview_bounds["max_x"]
        min_z_mm = self.preview_bounds["min_z"]
        max_z_mm = self.preview_bounds["max_z"]
        plot_width = side_right - side_left - 36
        plot_height = side_bottom - side_top - 34
        plot_scale = min(plot_width / (max_x_mm - min_x_mm), plot_height / (max_z_mm - min_z_mm))
        centered_plot_left = side_left + 18 + ((plot_width - ((max_x_mm - min_x_mm) * plot_scale)) * 0.5)
        centered_plot_bottom = side_bottom - 16

        def side_x(value_mm: float) -> float:
            return centered_plot_left + ((value_mm - min_x_mm) * plot_scale)

        def side_z(value_mm: float) -> float:
            return centered_plot_bottom - ((value_mm - min_z_mm) * plot_scale)

        def segment_polygon(x0: float, y0: float, x1: float, y1: float, half_width: float) -> list[float]:
            dx = x1 - x0
            dy = y1 - y0
            length = math.hypot(dx, dy)
            if length <= 1e-6:
                return [
                    x0 - half_width, y0 - half_width,
                    x0 + half_width, y0 - half_width,
                    x0 + half_width, y0 + half_width,
                    x0 - half_width, y0 + half_width,
                ]

            normal_x = -dy / length
            normal_y = dx / length
            return [
                x0 + (normal_x * half_width), y0 + (normal_y * half_width),
                x1 + (normal_x * half_width), y1 + (normal_y * half_width),
                x1 - (normal_x * half_width), y1 - (normal_y * half_width),
                x0 - (normal_x * half_width), y0 - (normal_y * half_width),
            ]

        def add_callout(anchor_x: float, anchor_y: float, label_x: float, label_y: float, title: str, body: str, accent: str) -> None:
            canvas.create_line(anchor_x, anchor_y, label_x, label_y, fill=accent, width=2)
            text_id = canvas.create_text(
                label_x + 6,
                label_y,
                anchor="w",
                text=f"{title}\n{body}",
                fill="#dcecff",
                font=("Segoe UI", 8, "bold"),
                justify="left",
            )
            left, top, right, bottom = canvas.bbox(text_id)
            canvas.create_rectangle(left - 6, top - 4, right + 6, bottom + 4, fill="#0f2032", outline=accent, width=1)
            canvas.tag_raise(text_id)

        def draw_outlined_polyline(points: list[float], outer_width: float, inner_width: float, outline: str, fill: str) -> None:
            canvas.create_line(*points, fill=outline, width=outer_width, capstyle=tk.ROUND, joinstyle=tk.ROUND)
            canvas.create_line(*points, fill=fill, width=inner_width, capstyle=tk.ROUND, joinstyle=tk.ROUND)

        def draw_joint(center_x: float, center_y: float, outer_radius: float, inner_radius: float, outline: str, fill: str) -> None:
            canvas.create_oval(
                center_x - outer_radius,
                center_y - outer_radius,
                center_x + outer_radius,
                center_y + outer_radius,
                fill=fill,
                outline=outline,
                width=4,
            )
            if inner_radius > 0.0:
                canvas.create_oval(
                    center_x - inner_radius,
                    center_y - inner_radius,
                    center_x + inner_radius,
                    center_y + inner_radius,
                    fill=fill,
                    outline=outline,
                    width=3,
                )

        panel_fill = "#d0d4d8"
        arm_fill = "#eceeef"
        outline_color = "#2f3135"
        accent_shadow = "#a8afb5"
        zero_reference_y = side_z(0.0)
        base_x = side_x(0.0)
        shoulder_y = side_z(ARM_BASE_HEIGHT_MM)
        elbow_x = side_x(pose["elbow_x"])
        elbow_y = side_z(pose["elbow_z"])
        wrist_x = side_x(pose["wrist_x"])
        wrist_y = side_z(pose["wrist_z"])
        base_plate_bottom_y = side_bottom - 6.0
        base_plate_top_y = base_plate_bottom_y - 10.0
        shoulder_joint_outer_radius = 24.0
        shoulder_draw_y = shoulder_y
        pedestal_top_y = shoulder_draw_y + shoulder_joint_outer_radius - 2.0
        pedestal_bottom_y = base_plate_top_y

        canvas.create_rectangle(side_left + 8, side_top + 8, side_right - 8, side_bottom - 8, fill=panel_fill, outline="")
        canvas.create_line(side_left + 12, zero_reference_y, side_right - 12, zero_reference_y, fill=accent_shadow, width=1, dash=(6, 5))
        canvas.create_text(side_left + 20, side_top + 18, anchor="w", text="Side View", fill=outline_color, font=("Segoe UI", 10, "bold"))
        canvas.create_text(side_right - 16, zero_reference_y - 10, anchor="e", text="Z=0", fill="#66707a", font=("Segoe UI", 8, "bold"))

        canvas.create_polygon(
            base_x - 22,
            pedestal_top_y,
            base_x + 10,
            pedestal_top_y,
            base_x + 18,
            pedestal_bottom_y,
            base_x - 30,
            pedestal_bottom_y,
            fill=arm_fill,
            outline=outline_color,
            width=4,
        )
        canvas.create_rectangle(
            base_x - 46,
            base_plate_top_y,
            base_x + 42,
            base_plate_bottom_y,
            fill=arm_fill,
            outline=outline_color,
            width=4,
        )
        draw_outlined_polyline(
            [base_x + 10.0, shoulder_draw_y + 8.0, base_x + 14.0, shoulder_draw_y - 12.0, elbow_x - 8.0, elbow_y + 10.0],
            outer_width=15.0,
            inner_width=9.0,
            outline=outline_color,
            fill=arm_fill,
        )
        draw_outlined_polyline(
            [elbow_x + 10.0, elbow_y + 2.0, wrist_x - 10.0, wrist_y + 4.0],
            outer_width=14.0,
            inner_width=8.0,
            outline=outline_color,
            fill=arm_fill,
        )

        draw_joint(base_x, shoulder_draw_y, outer_radius=shoulder_joint_outer_radius, inner_radius=16.0, outline=outline_color, fill=arm_fill)
        draw_joint(elbow_x, elbow_y, outer_radius=18.0, inner_radius=11.0, outline=outline_color, fill=arm_fill)
        draw_joint(wrist_x, wrist_y, outer_radius=13.0, inner_radius=0.0, outline=outline_color, fill=arm_fill)

        clamp_ratio = (self.clamp_var.get() - CLAMP_OPEN_PULSE) / (CLAMP_CLOSE_PULSE - CLAMP_OPEN_PULSE)
        clamp_ratio = max(0.0, min(1.0, clamp_ratio))
        jaw_spread = 22.0 - (12.0 * clamp_ratio)
        tool_angle = pose["tool_angle"]
        tool_dx = math.cos(tool_angle)
        tool_dz = math.sin(tool_angle)
        normal_dx = -tool_dz
        normal_dz = tool_dx
        claw_mount_x = wrist_x + (tool_dx * 8.0)
        claw_mount_y = wrist_y - (tool_dz * 8.0)
        jaw1_base_x = claw_mount_x + (normal_dx * jaw_spread * 0.5)
        jaw1_base_y = claw_mount_y - (normal_dz * jaw_spread * 0.5)
        jaw2_base_x = claw_mount_x - (normal_dx * jaw_spread * 0.5)
        jaw2_base_y = claw_mount_y + (normal_dz * jaw_spread * 0.5)
        jaw1_mid_x = jaw1_base_x + (tool_dx * 12.0)
        jaw1_mid_y = jaw1_base_y - (tool_dz * 12.0)
        jaw2_mid_x = jaw2_base_x + (tool_dx * 12.0)
        jaw2_mid_y = jaw2_base_y - (tool_dz * 12.0)
        jaw1_tip_x = jaw1_mid_x + (tool_dx * 10.0) + (normal_dx * 8.0)
        jaw1_tip_y = jaw1_mid_y - (tool_dz * 10.0) - (normal_dz * 8.0)
        jaw2_tip_x = jaw2_mid_x + (tool_dx * 10.0) - (normal_dx * 8.0)
        jaw2_tip_y = jaw2_mid_y - (tool_dz * 10.0) + (normal_dz * 8.0)

        draw_outlined_polyline(
            [wrist_x, wrist_y, claw_mount_x, claw_mount_y],
            outer_width=10.0,
            inner_width=5.0,
            outline=outline_color,
            fill=arm_fill,
        )
        draw_outlined_polyline(
            [jaw1_base_x, jaw1_base_y, jaw1_mid_x, jaw1_mid_y, jaw1_tip_x, jaw1_tip_y],
            outer_width=8.0,
            inner_width=4.0,
            outline=outline_color,
            fill=arm_fill,
        )
        draw_outlined_polyline(
            [jaw2_base_x, jaw2_base_y, jaw2_mid_x, jaw2_mid_y, jaw2_tip_x, jaw2_tip_y],
            outer_width=8.0,
            inner_width=4.0,
            outline=outline_color,
            fill=arm_fill,
        )

        add_callout(base_x + 18, shoulder_draw_y - 12, side_left + 88, side_top + 38, "Fore/Aft", "Front/back joint", "#ffb347")
        add_callout(elbow_x + 10, elbow_y + 2, side_right - 112, side_top + 86, "Gripper Lift", "Up/down joint", "#65b9ff")

        if target is not None:
            target_x = side_x(float(target[0]))
            target_y = side_z(float(target[2]))
            canvas.create_oval(target_x - 10, target_y - 10, target_x + 10, target_y + 10, outline="#ff6f91", width=3)
            canvas.create_line(target_x - 16, target_y, target_x + 16, target_y, fill="#ff6f91", width=2)
            canvas.create_line(target_x, target_y - 16, target_x, target_y + 16, fill="#ff6f91", width=2)

        tof_panel_left = left_col_left
        tof_panel_right = left_col_right
        tof_panel_top = top_row_top
        tof_panel_bottom = top_row_bottom
        if (tof_panel_right - tof_panel_left) >= 170.0:
            tof_panel_width = tof_panel_right - tof_panel_left
            tof_panel_height = tof_panel_bottom - tof_panel_top
            tof_center_x = tof_panel_left + (tof_panel_width * 0.5)
            tof_center_y = tof_panel_top + min(126.0, tof_panel_height * 0.42)
            gauge_radius = min(74.0, tof_panel_width * 0.34)
            poll_interval_ms = self._get_status_poll_interval_ms()
            gauge_left = tof_center_x - gauge_radius
            gauge_top = tof_center_y - gauge_radius
            gauge_right = tof_center_x + gauge_radius
            gauge_bottom = tof_center_y + gauge_radius
            display_cap_mm = max(300, self._obstacle_clear_mm * 2, self._obstacle_range_mm + 40)
            obstacle_ratio = max(0.0, min(1.0, self._obstacle_range_mm / display_cap_mm)) if display_cap_mm > 0 else 0.0
            clear_ratio = max(0.0, min(1.0, self._obstacle_clear_mm / display_cap_mm)) if display_cap_mm > 0 else 0.0

            if self._obstacle_monitor_state == "detected":
                tof_primary = "#ff6b6b"
                tof_secondary = "#ffd0c8"
                tof_status_title = "PATH BLOCKED"
                tof_status_body = "TOF auto-brake engaged"
                tof_card_fill = "#0f1726"
                tof_card_outline = "#913838"
            elif self._obstacle_monitor_state == "clear":
                tof_primary = "#52f7ff"
                tof_secondary = "#d8fdff"
                tof_status_title = "CLEAR TO MOVE"
                tof_status_body = "3D pick feels safe"
                tof_card_fill = "#0f1726"
                tof_card_outline = "#226a7c"
            elif self._obstacle_monitor_state == "disabled":
                tof_primary = "#7e8ca3"
                tof_secondary = "#d5dbe4"
                tof_status_title = "TOF OFFLINE"
                tof_status_body = "Connect and poll STATUS"
                tof_card_fill = "#111722"
                tof_card_outline = "#3b4858"
            else:
                tof_primary = "#ffb347"
                tof_secondary = "#fff0c7"
                tof_status_title = "TOF STANDBY"
                tof_status_body = "Waiting for valid range"
                tof_card_fill = "#0f1726"
                tof_card_outline = "#7a5a24"

            canvas.create_rectangle(tof_panel_left, tof_panel_top, tof_panel_right, tof_panel_bottom, fill=tof_card_fill, outline=tof_card_outline, width=2)
            canvas.create_oval(tof_panel_left - 26, tof_panel_top + 18, tof_panel_left + 64, tof_panel_top + 108, fill="#13283f", outline="")
            canvas.create_oval(tof_panel_right - 86, tof_panel_top + 132, tof_panel_right + 8, tof_panel_top + 226, fill="#10233a", outline="")
            canvas.create_text(
                tof_panel_left + 18,
                tof_panel_top + 18,
                anchor="w",
                text="TOF Safety",
                fill="#f3fbff",
                font=("Segoe UI", 12, "bold"),
            )
            canvas.create_text(
                tof_panel_left + 18,
                tof_panel_top + 40,
                anchor="w",
                text="3D pick + obstacle auto-brake",
                fill="#8bcfff",
                font=("Segoe UI", 8, "bold"),
            )
            canvas.create_arc(gauge_left, gauge_top, gauge_right, gauge_bottom, start=135, extent=270, style=tk.ARC, width=16, outline="#173149")
            canvas.create_arc(
                gauge_left,
                gauge_top,
                gauge_right,
                gauge_bottom,
                start=135,
                extent=270 * clear_ratio,
                style=tk.ARC,
                width=16,
                outline="#6f2d34",
            )
            canvas.create_arc(
                gauge_left,
                gauge_top,
                gauge_right,
                gauge_bottom,
                start=135,
                extent=270 * obstacle_ratio,
                style=tk.ARC,
                width=16,
                outline=tof_primary,
            )
            marker_angle_deg = 135 + (270 * min(1.0, max(0.0, self._obstacle_threshold_mm / display_cap_mm)))
            marker_angle_rad = math.radians(marker_angle_deg)
            marker_outer_radius = gauge_radius + 7.0
            marker_inner_radius = gauge_radius - 13.0
            marker_x_outer = tof_center_x + (math.cos(marker_angle_rad) * marker_outer_radius)
            marker_y_outer = tof_center_y - (math.sin(marker_angle_rad) * marker_outer_radius)
            marker_x_inner = tof_center_x + (math.cos(marker_angle_rad) * marker_inner_radius)
            marker_y_inner = tof_center_y - (math.sin(marker_angle_rad) * marker_inner_radius)
            canvas.create_line(marker_x_inner, marker_y_inner, marker_x_outer, marker_y_outer, fill="#ffe082", width=3)
            canvas.create_text(
                tof_center_x,
                tof_center_y - 8,
                text=f"{self._obstacle_range_mm if self._obstacle_monitor_state != 'disabled' else '--'}",
                fill=tof_secondary,
                font=("Segoe UI", 24, "bold"),
            )
            canvas.create_text(
                tof_center_x,
                tof_center_y + 20,
                text="mm",
                fill="#94abc2",
                font=("Segoe UI", 10, "bold"),
            )
            canvas.create_text(
                tof_center_x,
                tof_center_y + 48,
                text=tof_status_title,
                fill=tof_primary,
                font=("Segoe UI", 10, "bold"),
            )
            canvas.create_text(
                tof_center_x,
                tof_center_y + 68,
                text=tof_status_body,
                fill="#d6e7f8",
                font=("Segoe UI", 8),
            )
            sensor_bar_left = tof_panel_left + 24
            sensor_bar_right = tof_panel_right - 24
            sensor_bar_top = tof_panel_bottom - 92
            sensor_bar_bottom = sensor_bar_top + 16
            sensor_fill_right = sensor_bar_left + ((sensor_bar_right - sensor_bar_left) * obstacle_ratio)
            canvas.create_rectangle(sensor_bar_left, sensor_bar_top, sensor_bar_right, sensor_bar_bottom, fill="#102033", outline="#264764")
            canvas.create_rectangle(sensor_bar_left, sensor_bar_top, sensor_fill_right, sensor_bar_bottom, fill=tof_primary, outline="")
            threshold_x = sensor_bar_left + ((sensor_bar_right - sensor_bar_left) * min(1.0, max(0.0, self._obstacle_threshold_mm / display_cap_mm)))
            canvas.create_line(threshold_x, sensor_bar_top - 6, threshold_x, sensor_bar_bottom + 6, fill="#ffe082", width=2)
            canvas.create_text(
                sensor_bar_left,
                sensor_bar_top - 10,
                anchor="sw",
                text=f"Auto-stop <= {self._obstacle_threshold_mm} mm",
                fill="#ffe082",
                font=("Segoe UI", 8, "bold"),
            )
            canvas.create_text(
                sensor_bar_left,
                sensor_bar_bottom + 12,
                anchor="nw",
                text=f"update {poll_interval_ms} ms",
                fill="#7ca2c4",
                font=("Segoe UI", 7),
            )
            canvas.create_text(
                sensor_bar_right,
                sensor_bar_bottom + 12,
                anchor="ne",
                text=f"device {self._obstacle_device_name}",
                fill="#7ca2c4",
                font=("Segoe UI", 7),
            )
            cta_left = tof_panel_left + 18
            cta_right = tof_panel_right - 18
            cta_top = tof_panel_bottom - 48
            cta_bottom = tof_panel_bottom - 18
            canvas.create_rectangle(cta_left, cta_top, cta_right, cta_bottom, fill="#11314c", outline="#50d7ff", width=2)
            canvas.create_text(
                cta_left + 12,
                (cta_top + cta_bottom) * 0.5,
                anchor="w",
                text="Click 3D Cloud",
                fill="#ecfcff",
                font=("Segoe UI", 10, "bold"),
            )
            canvas.create_text(
                cta_right - 12,
                (cta_top + cta_bottom) * 0.5,
                anchor="e",
                text=">>>",
                fill="#50d7ff",
                font=("Consolas", 12, "bold"),
            )

        imu_panel_left = right_col_left
        imu_panel_right = right_col_right
        imu_panel_top = top_row_top
        imu_panel_bottom = top_row_bottom

        if (imu_panel_right - imu_panel_left) >= 170.0:
            imu_panel_width = imu_panel_right - imu_panel_left
            imu_panel_height = imu_panel_bottom - imu_panel_top
            imu_center_x = imu_panel_left + (imu_panel_width * 0.5)
            disk_center_y = imu_panel_top + 88
            disk_radius = max(28.0, min(46.0, imu_panel_height * 0.21))

            if self._imu_monitor_state == "level":
                imu_primary = "#52f7ff"
                imu_status_title = "LEVEL"
                imu_card_outline = "#226a7c"
                imu_card_fill = "#0f1726"
            elif self._imu_monitor_state == "tilted":
                imu_primary = "#ffb347"
                imu_status_title = "TILTED"
                imu_card_outline = "#7a5a24"
                imu_card_fill = "#0f1726"
            elif self._imu_monitor_state == "shaking":
                imu_primary = "#ff6b6b"
                imu_status_title = "SHAKING"
                imu_card_outline = "#913838"
                imu_card_fill = "#0f1726"
            elif self._imu_monitor_state == "disabled":
                imu_primary = "#7e8ca3"
                imu_status_title = "IMU OFFLINE"
                imu_card_outline = "#3b4858"
                imu_card_fill = "#111722"
            else:
                imu_primary = "#ffb347"
                imu_status_title = "IMU STANDBY"
                imu_card_outline = "#7a5a24"
                imu_card_fill = "#0f1726"

            canvas.create_rectangle(imu_panel_left, imu_panel_top, imu_panel_right, imu_panel_bottom, fill=imu_card_fill, outline=imu_card_outline, width=2)
            canvas.create_oval(imu_panel_right - 88, imu_panel_top + 14, imu_panel_right + 8, imu_panel_top + 110, fill="#13283f", outline="")
            canvas.create_oval(imu_panel_left - 24, imu_panel_top + 134, imu_panel_left + 80, imu_panel_top + 230, fill="#10233a", outline="")

            canvas.create_text(
                imu_panel_left + 18,
                imu_panel_top + 18,
                anchor="w",
                text="IMU Attitude",
                fill="#f3fbff",
                font=("Segoe UI", 12, "bold"),
            )
            canvas.create_text(
                imu_panel_left + 18,
                imu_panel_top + 40,
                anchor="w",
                text="gripper pitch + roll",
                fill="#8bcfff",
                font=("Segoe UI", 8, "bold"),
            )

            if self._imu_monitor_state == "disabled":
                canvas.create_oval(imu_center_x - disk_radius, disk_center_y - disk_radius, imu_center_x + disk_radius, disk_center_y + disk_radius, fill="#1a2231", outline=imu_card_outline, width=2)
                canvas.create_text(imu_center_x, disk_center_y, text="OFFLINE", fill="#7e8ca3", font=("Segoe UI", 9, "bold"))
            else:
                pitch_deg_visual = max(-90, min(90, int(self._imu_pitch_deg)))
                roll_deg_visual = int(self._imu_roll_deg)
                pitch_offset = max(-disk_radius, min(disk_radius, (pitch_deg_visual / 45.0) * disk_radius * 0.6))
                roll_rad_view = math.radians(roll_deg_visual)
                cos_r = math.cos(-roll_rad_view)
                sin_r = math.sin(-roll_rad_view)

                canvas.create_oval(imu_center_x - disk_radius, disk_center_y - disk_radius, imu_center_x + disk_radius, disk_center_y + disk_radius, fill="#3da4e0", outline="")

                if pitch_offset >= disk_radius - 0.5:
                    canvas.create_oval(imu_center_x - disk_radius, disk_center_y - disk_radius, imu_center_x + disk_radius, disk_center_y + disk_radius, fill="#c47a3e", outline="")
                elif pitch_offset > -disk_radius + 0.5:
                    n_arc = 32
                    t_lo = math.asin(pitch_offset / disk_radius)
                    t_hi = math.pi - t_lo
                    polygon_points: list[float] = []
                    for i in range(n_arc + 1):
                        t = t_lo + (t_hi - t_lo) * (i / n_arc)
                        lx = disk_radius * math.cos(t)
                        ly = disk_radius * math.sin(t)
                        sx = lx * cos_r - ly * sin_r
                        sy = lx * sin_r + ly * cos_r
                        polygon_points.append(imu_center_x + sx)
                        polygon_points.append(disk_center_y + sy)
                    canvas.create_polygon(*polygon_points, fill="#c47a3e", outline="")

                if -disk_radius < pitch_offset < disk_radius:
                    dx_h = math.sqrt(disk_radius * disk_radius - pitch_offset * pitch_offset)
                    x1 = -dx_h * cos_r - pitch_offset * sin_r
                    y1 = -dx_h * sin_r + pitch_offset * cos_r
                    x2 = dx_h * cos_r - pitch_offset * sin_r
                    y2 = dx_h * sin_r + pitch_offset * cos_r
                    canvas.create_line(imu_center_x + x1, disk_center_y + y1, imu_center_x + x2, disk_center_y + y2, fill="#ffffff", width=2)

                for p_deg, dx_half in ((-30, disk_radius * 0.32), (-15, disk_radius * 0.22), (15, disk_radius * 0.22), (30, disk_radius * 0.32)):
                    ladder_offset = pitch_offset - (p_deg / 45.0) * disk_radius * 0.6
                    if abs(ladder_offset) >= disk_radius - 4:
                        continue
                    for sign in (-1, 1):
                        lx_a = sign * dx_half - 4
                        lx_b = sign * dx_half + 4
                        sx_a = lx_a * cos_r - ladder_offset * sin_r
                        sy_a = lx_a * sin_r + ladder_offset * cos_r
                        sx_b = lx_b * cos_r - ladder_offset * sin_r
                        sy_b = lx_b * sin_r + ladder_offset * cos_r
                        canvas.create_line(imu_center_x + sx_a, disk_center_y + sy_a, imu_center_x + sx_b, disk_center_y + sy_b, fill="#e0eef5", width=1)

                canvas.create_line(imu_center_x - disk_radius * 0.55, disk_center_y, imu_center_x - disk_radius * 0.18, disk_center_y, fill="#ffe082", width=3)
                canvas.create_line(imu_center_x + disk_radius * 0.18, disk_center_y, imu_center_x + disk_radius * 0.55, disk_center_y, fill="#ffe082", width=3)
                canvas.create_oval(imu_center_x - 3, disk_center_y - 3, imu_center_x + 3, disk_center_y + 3, fill="#ffe082", outline="")

                canvas.create_polygon(
                    imu_center_x, disk_center_y - disk_radius - 1,
                    imu_center_x - 5, disk_center_y - disk_radius - 9,
                    imu_center_x + 5, disk_center_y - disk_radius - 9,
                    fill="#ffe082", outline="",
                )
                roll_marker_angle = math.radians(-90 - roll_deg_visual)
                marker_x = imu_center_x + (disk_radius - 8) * math.cos(roll_marker_angle)
                marker_y = disk_center_y + (disk_radius - 8) * math.sin(roll_marker_angle)
                canvas.create_oval(marker_x - 4, marker_y - 4, marker_x + 4, marker_y + 4, fill=imu_primary, outline="#ffffff")
                canvas.create_oval(imu_center_x - disk_radius, disk_center_y - disk_radius, imu_center_x + disk_radius, disk_center_y + disk_radius, outline=imu_card_outline, width=2)

            if self._imu_monitor_state == "disabled":
                pose_text = "pitch  --°    roll  --°"
            else:
                pose_text = f"pitch {self._imu_pitch_deg:+d}°    roll {self._imu_roll_deg:+d}°"

            canvas.create_text(
                imu_center_x,
                imu_panel_top + 150,
                text=pose_text,
                fill="#dcecff",
                font=("Segoe UI", 13, "bold"),
            )

            gyro_bar_left = imu_panel_left + 24
            gyro_bar_right = imu_panel_right - 24
            gyro_bar_top = imu_panel_top + 172
            gyro_bar_bottom = gyro_bar_top + 8
            shaking_threshold_mdps = 8000
            display_cap_mdps = max(shaking_threshold_mdps * 2, self._imu_gyro_mag_mdps + 1500)
            gyro_ratio = max(0.0, min(1.0, self._imu_gyro_mag_mdps / display_cap_mdps)) if display_cap_mdps > 0 else 0.0
            gyro_fill_right = gyro_bar_left + ((gyro_bar_right - gyro_bar_left) * gyro_ratio)
            canvas.create_rectangle(gyro_bar_left, gyro_bar_top, gyro_bar_right, gyro_bar_bottom, fill="#102033", outline="#264764")
            if self._imu_monitor_state != "disabled":
                canvas.create_rectangle(gyro_bar_left, gyro_bar_top, gyro_fill_right, gyro_bar_bottom, fill=imu_primary, outline="")
            shaking_marker_x = gyro_bar_left + ((gyro_bar_right - gyro_bar_left) * min(1.0, shaking_threshold_mdps / display_cap_mdps))
            canvas.create_line(shaking_marker_x, gyro_bar_top - 4, shaking_marker_x, gyro_bar_bottom + 4, fill="#ffe082", width=2)

            if self._imu_monitor_state == "disabled":
                gyro_label = "ω -- mdps"
                temp_label = "T --.- °C"
            else:
                gyro_label = f"ω {self._imu_gyro_mag_mdps} mdps"
                temp_value = self._imu_temp_dc / 10.0
                temp_label = f"T {temp_value:+.1f} °C" if temp_value < 0 else f"T {temp_value:.1f} °C"

            canvas.create_text(
                gyro_bar_left,
                gyro_bar_bottom + 4,
                anchor="nw",
                text=f"Shake>{shaking_threshold_mdps} mdps",
                fill="#ffe082",
                font=("Segoe UI", 7, "bold"),
            )
            canvas.create_text(
                gyro_bar_right,
                gyro_bar_bottom + 4,
                anchor="ne",
                text=gyro_label,
                fill="#7ca2c4",
                font=("Segoe UI", 7),
            )

            imu_cta_left = imu_panel_left + 18
            imu_cta_right = imu_panel_right - 18
            imu_cta_top = imu_panel_bottom - 48
            imu_cta_bottom = imu_panel_bottom - 18
            canvas.create_rectangle(imu_cta_left, imu_cta_top, imu_cta_right, imu_cta_bottom, fill="#11314c", outline=imu_primary, width=2)
            canvas.create_text(
                imu_cta_left + 12,
                (imu_cta_top + imu_cta_bottom) * 0.5,
                anchor="w",
                text=imu_status_title,
                fill=imu_primary,
                font=("Segoe UI", 10, "bold"),
            )
            canvas.create_text(
                imu_cta_right - 12,
                (imu_cta_top + imu_cta_bottom) * 0.5,
                anchor="e",
                text=f"{self._imu_device_name}    {temp_label}",
                fill="#ecfcff",
                font=("Segoe UI", 8, "bold"),
            )

        workspace_left = right_col_left
        workspace_top = bottom_row_top
        workspace_right = right_col_right
        workspace_bottom = bottom_row_bottom
        workspace_width = workspace_right - workspace_left
        workspace_height = workspace_bottom - workspace_top
        workspace_text_width = workspace_width - 28
        self._workspace_panel_bounds = (workspace_left, workspace_top, workspace_right, workspace_bottom)
        canvas.create_rectangle(workspace_left, workspace_top, workspace_right, workspace_bottom, fill="#0d1628", outline="#233a57", width=2)
        canvas.create_text(workspace_left + 14, workspace_top + 18, anchor="w", text="3D Workspace", fill="#d8f3ff", font=("Segoe UI", 10, "bold"))
        badge_right = workspace_right - 14
        badge_left = badge_right - 112
        badge_top = workspace_top + 10
        badge_bottom = badge_top + 22
        canvas.create_rectangle(badge_left, badge_top, badge_right, badge_bottom, fill="#12324a", outline="#4fd8ff", width=1)
        canvas.create_text(
            (badge_left + badge_right) * 0.5,
            (badge_top + badge_bottom) * 0.5,
            text="TRY 3D PICK",
            fill="#ecfcff",
            font=("Segoe UI", 8, "bold"),
        )
        workspace_header_text_width = max(160.0, (badge_left - workspace_left) - 28.0)
        canvas.create_text(
            workspace_left + 14,
            workspace_top + 36,
            anchor="w",
            text="Hover shows nearest XYZ. Click the cyan cloud to preview motion, release to send, and right-drag to rotate the 3D view.",
            fill="#8bcfff",
            font=("Segoe UI", 8),
            width=workspace_header_text_width,
        )

        workspace_plot_left = workspace_left + 14
        workspace_plot_top = workspace_top + 70
        workspace_plot_right = workspace_right - 14
        workspace_plot_bottom = workspace_bottom - 84
        workspace_plot_width = workspace_plot_right - workspace_plot_left
        workspace_plot_height = workspace_plot_bottom - workspace_plot_top
        self.workspace_projection_bounds = self._estimate_workspace_projection_bounds(self.workspace_preview_points)
        projected_bounds = self.workspace_projection_bounds
        projected_width = projected_bounds["max_x"] - projected_bounds["min_x"]
        projected_height = projected_bounds["max_y"] - projected_bounds["min_y"]
        workspace_scale = min(workspace_plot_width / projected_width, workspace_plot_height / projected_height)
        render_width = projected_width * workspace_scale
        render_height = projected_height * workspace_scale
        render_left = workspace_plot_left + ((workspace_plot_width - render_width) * 0.5)
        render_top = workspace_plot_top + ((workspace_plot_height - render_height) * 0.5)

        def workspace_screen_point(x_mm: float, y_mm: float, z_mm: float) -> tuple[float, float]:
            projected_x, projected_y = self._project_workspace_point(x_mm, y_mm, z_mm)
            screen_x = render_left + ((projected_x - projected_bounds["min_x"]) * workspace_scale)
            screen_y = render_top + render_height - ((projected_y - projected_bounds["min_y"]) * workspace_scale)
            return screen_x, screen_y

        origin_x, origin_y = workspace_screen_point(0.0, 0.0, 0.0)
        axis_guide_radius_mm = self.workspace_bounds["max_radius"] * WORKSPACE_AXIS_GUIDE_RATIO
        axis_guide_height_mm = self.workspace_bounds["max_z"] * WORKSPACE_AXIS_GUIDE_RATIO
        x_axis_end_x, x_axis_end_y = workspace_screen_point(axis_guide_radius_mm, 0.0, 0.0)
        y_axis_end_x, y_axis_end_y = workspace_screen_point(0.0, axis_guide_radius_mm, 0.0)
        z_axis_end_x, z_axis_end_y = workspace_screen_point(0.0, 0.0, axis_guide_height_mm)

        canvas.create_line(origin_x, origin_y, x_axis_end_x, x_axis_end_y, fill="#5fd0ff", width=2)
        canvas.create_line(origin_x, origin_y, y_axis_end_x, y_axis_end_y, fill="#4da3ff", width=2)
        canvas.create_line(origin_x, origin_y, z_axis_end_x, z_axis_end_y, fill="#9de37d", width=2)
        canvas.create_text(x_axis_end_x + 6, x_axis_end_y, anchor="w", text="X", fill="#5fd0ff", font=("Segoe UI", 8, "bold"))
        canvas.create_text(y_axis_end_x - 6, y_axis_end_y, anchor="e", text="Y", fill="#4da3ff", font=("Segoe UI", 8, "bold"))
        canvas.create_text(z_axis_end_x, z_axis_end_y - 8, anchor="s", text="Z", fill="#9de37d", font=("Segoe UI", 8, "bold"))

        hover_workspace_x = None
        hover_workspace_y = None
        drag_workspace_x = None
        drag_workspace_y = None
        for workspace_point in self.workspace_preview_points:
            point_x, point_y = workspace_screen_point(float(workspace_point[0]), float(workspace_point[1]), float(workspace_point[2]))
            depth_ratio = 0.0
            if self.workspace_bounds["max_z"] > SAFE_WORKSPACE_MIN_Z_MM:
                depth_ratio = (workspace_point[2] - SAFE_WORKSPACE_MIN_Z_MM) / (self.workspace_bounds["max_z"] - SAFE_WORKSPACE_MIN_Z_MM)
            point_color = "#43d3ff" if depth_ratio < 0.5 else "#8bf7ff"
            point_radius = 2.0 if depth_ratio < 0.7 else 2.6
            canvas.create_oval(point_x - point_radius, point_y - point_radius, point_x + point_radius, point_y + point_radius, fill=point_color, outline="")
            if self._workspace_hover_target == workspace_point:
                hover_workspace_x = point_x
                hover_workspace_y = point_y
            if self._workspace_drag_target == workspace_point:
                drag_workspace_x = point_x
                drag_workspace_y = point_y
            self._workspace_click_candidates.append((point_x, point_y, workspace_point))

        if hover_workspace_x is not None and hover_workspace_y is not None:
            canvas.create_oval(hover_workspace_x - 6, hover_workspace_y - 6, hover_workspace_x + 6, hover_workspace_y + 6, outline="#ffffff", width=1)

        if drag_workspace_x is not None and drag_workspace_y is not None:
            canvas.create_oval(drag_workspace_x - 9, drag_workspace_y - 9, drag_workspace_x + 9, drag_workspace_y + 9, outline="#ff9f6b", width=2)
            canvas.create_line(drag_workspace_x - 12, drag_workspace_y, drag_workspace_x + 12, drag_workspace_y, fill="#ff9f6b", width=2)
            canvas.create_line(drag_workspace_x, drag_workspace_y - 12, drag_workspace_x, drag_workspace_y + 12, fill="#ff9f6b", width=2)

        actual_workspace_x, actual_workspace_y = workspace_screen_point(pose["x"], pose["y"], pose["z"])
        canvas.create_line(origin_x, origin_y, actual_workspace_x, actual_workspace_y, fill="#ffe082", width=3, capstyle=tk.ROUND)
        canvas.create_oval(actual_workspace_x - 6, actual_workspace_y - 6, actual_workspace_x + 6, actual_workspace_y + 6, fill="#ffe082", outline="#fff8d0")

        if target is not None:
            target_workspace_x, target_workspace_y = workspace_screen_point(float(target[0]), float(target[1]), float(target[2]))
            canvas.create_oval(target_workspace_x - 8, target_workspace_y - 8, target_workspace_x + 8, target_workspace_y + 8, outline="#ff5b8f", width=3)
            canvas.create_line(target_workspace_x - 12, target_workspace_y, target_workspace_x + 12, target_workspace_y, fill="#ff5b8f", width=2)
            canvas.create_line(target_workspace_x, target_workspace_y - 12, target_workspace_x, target_workspace_y + 12, fill="#ff5b8f", width=2)

        canvas.create_text(
            workspace_left + 14,
            workspace_bottom - 78,
            anchor="nw",
            text=f"Actual x={pose['x']:.0f} y={pose['y']:.0f} z={pose['z']:.0f} mm",
            fill="#a9dcff",
            font=("Segoe UI", 10, "bold"),
            width=workspace_text_width,
        )
        canvas.create_text(
            workspace_left + 14,
            workspace_bottom - 54,
            anchor="nw",
            text=(
                f"Preview target x={target[0]} y={target[1]} z={target[2]} mm"
                if target is not None
                else f"Yaw {math.degrees(yaw_rad):.0f} deg | Reach {pose['radial']:.0f} mm"
            ),
            fill="#c8dcf4",
            font=("Segoe UI", 9),
            width=workspace_text_width,
        )
        canvas.create_text(
            workspace_left + 14,
            workspace_bottom - 30,
            anchor="nw",
            text=(
                f"Hover x={self._workspace_hover_target[0]} y={self._workspace_hover_target[1]} z={self._workspace_hover_target[2]} mm"
                if self._workspace_hover_target is not None
                else "Click a cyan point | hold preview or switch to Live Follow | right-drag orbit"
            ),
            fill="#8bcfff",
            font=("Segoe UI", 8),
            width=workspace_text_width,
        )

    def refresh_ports(self) -> None:
        comports = list_ports.comports()
        # 顯示端口名稱和描述
        port_display_list = []
        port_mapping = {}
        for port in sorted(comports, key=lambda p: p.device):
            display_name = f"{port.device} - {port.description}" if port.description else port.device
            port_display_list.append(display_name)
            port_mapping[display_name] = port.device

        self.port_combo["values"] = port_display_list
        self._port_mapping = port_mapping

        current_value = self.port_var.get()
        # 檢查當前值是否還有效
        if current_value not in port_mapping:
            if port_display_list:
                self.port_var.set(port_display_list[0])
            else:
                self.port_var.set("")
        if not port_display_list:
            self.port_var.set("")

    def toggle_connection(self) -> None:
        if self.serial_port is None:
            self.connect_serial()
        else:
            self.disconnect_serial()

    def connect_serial(self) -> None:
        port_display = self.port_var.get().strip()
        if not port_display:
            messagebox.showerror("Robot Arm Debug Tool", "Select a serial port first.")
            return

        # 從映射獲取實際端口
        port = getattr(self, '_port_mapping', {}).get(port_display, port_display)

        try:
            self.serial_port = serial.Serial(port=port, baudrate=115200, timeout=0.1, write_timeout=1.0)
        except SerialException as exc:
            messagebox.showerror("Robot Arm Debug Tool", f"Failed to open {port}: {exc}")
            return

        self.reader_stop_event.clear()
        self.reader_thread = threading.Thread(target=self._reader_loop, daemon=True)
        self.reader_thread.start()
        self.connection_var.set(f"Connected to {port} @ 115200")
        self.connect_button_var.set("Disconnect")
        self.firmware_motion_var.set("Firmware connected. Waiting for STATUS...")
        self.firmware_pose_var.set("Firmware pose pending")
        self.firmware_target_var.set("Firmware target pending")
        self._set_firmware_obstacle_feedback("Obstacle monitor waiting for STATUS...", "warning")
        self._set_obstacle_monitor_state("unknown", 0, 120, 140, "pending")
        self._set_imu_monitor_state("unknown", 0, 0, 0, 0, "pending")
        self._set_fw_command_feedback("FW command result: connected, waiting for next command", "FW echo: no command acknowledgement yet", tone="neutral")
        self.append_log(f"[host] connected to {port}")
        self._schedule_status_poll(initial_delay_ms=self._get_status_poll_interval_ms())

    def disconnect_serial(self) -> None:
        if self.serial_port is None:
            return

        self._cancel_status_poll()
        self.reader_stop_event.set()
        if self.reader_thread is not None:
            self.reader_thread.join(timeout=1.0)
            self.reader_thread = None

        try:
            self.serial_port.close()
        except SerialException:
            pass

        self.serial_port = None
        self.connection_var.set("Disconnected")
        self.connect_button_var.set("Connect")
        self.firmware_motion_var.set("Firmware motion idle | disconnected")
        self.firmware_pose_var.set("Firmware pose unavailable")
        self.firmware_target_var.set("Firmware target unavailable")
        self._set_firmware_obstacle_feedback("Obstacle monitor unavailable | disconnected", "neutral")
        self._set_obstacle_monitor_state("disabled", 0, 120, 140, "disconnected")
        self._set_imu_monitor_state("disabled", 0, 0, 0, 0, "disconnected")
        self._set_fw_command_feedback("FW command result: disconnected", "FW echo: serial link closed", tone="error")
        self.append_log("[host] disconnected")

    def _reader_loop(self) -> None:
        while not self.reader_stop_event.is_set():
            try:
                line = self.serial_port.readline()
            except SerialException as exc:
                self.log_queue.put(f"[host] serial read failed: {exc}")
                break

            if line:
                decoded = line.decode("utf-8", errors="replace").rstrip()
                self.log_queue.put(decoded)

    def _poll_log_queue(self) -> None:
        while True:
            try:
                line = self.log_queue.get_nowait()
            except queue.Empty:
                break
            else:
                self.append_log(line)

        self.after(LOG_QUEUE_POLL_INTERVAL_MS, self._poll_log_queue)

    def _append_log_text(self, line: str) -> None:
        self.log_text.configure(state="normal")
        self.log_text.insert("end", line + "\n")
        self.log_text.see("end")
        self.log_text.configure(state="disabled")

    def append_log(self, line: str) -> None:
        self._handle_firmware_line(line)
        if self.log_paused:
            self.paused_log_lines.append(line)
            return

        self._append_log_text(line)

    def toggle_log_pause(self) -> None:
        self.log_paused = not self.log_paused
        self.log_pause_button_var.set("Resume Log" if self.log_paused else "Pause Log")
        if self.log_paused or not self.paused_log_lines:
            return

        buffered_lines = self.paused_log_lines
        self.paused_log_lines = []
        for buffered_line in buffered_lines:
            self._append_log_text(buffered_line)

    def copy_log(self) -> None:
        log_text = self.log_text.get("1.0", "end-1c")
        if self.paused_log_lines:
            pending_text = "\n".join(self.paused_log_lines)
            log_text = pending_text if not log_text else f"{log_text}\n{pending_text}"

        self.clipboard_clear()
        self.clipboard_append(log_text)
        self.update_idletasks()

    def clear_log(self) -> None:
        self.paused_log_lines = []
        self.log_text.configure(state="normal")
        self.log_text.delete("1.0", "end")
        self.log_text.configure(state="disabled")

    def send_command(self, command: str) -> None:
        if self.serial_port is None:
            self._set_fw_command_feedback("FW command result: send failed", "FW echo: serial port is not connected", tone="error")
            messagebox.showerror("Robot Arm Debug Tool", "Serial port is not connected.")
            return

        self._write_serial_command(command, log_command=True)

    def _write_serial_command(self, command: str, log_command: bool) -> bool:
        if self.serial_port is None:
            return False

        payload = command.strip() + "\n"
        try:
            self.serial_port.write(payload.encode("ascii"))
        except SerialException as exc:
            self._set_fw_command_feedback("FW command result: send failed", f"FW echo: serial write failed: {exc}", tone="error")
            messagebox.showerror("Robot Arm Debug Tool", f"Failed to send command: {exc}")
            self.disconnect_serial()
            return False

        if log_command:
            self.append_log(f">> {command.strip()}")
        return True

    def _cancel_status_poll(self) -> None:
        if self.status_poll_after_id is not None:
            self.after_cancel(self.status_poll_after_id)
            self.status_poll_after_id = None

    def _schedule_status_poll(self, initial_delay_ms: int | None = None) -> None:
        self._cancel_status_poll()
        if self.serial_port is None:
            return

        delay_ms = self._get_status_poll_interval_ms() if initial_delay_ms is None else max(TOF_STATUS_POLL_INTERVAL_MIN_MS, int(initial_delay_ms))
        self.status_poll_after_id = self.after(delay_ms, self._request_status_poll)

    def _request_status_poll(self) -> None:
        self.status_poll_after_id = None
        if self.serial_port is None:
            return

        if self._write_serial_command("STATUS", log_command=False):
            self._schedule_status_poll()

    def _handle_firmware_line(self, line: str) -> None:
        obstacle_event_match = SAFETY_OBSTACLE_PATTERN.match(line)
        if obstacle_event_match is not None:
            state, range_mm, threshold_mm, _clear_mm, device_name, motion_name = obstacle_event_match.groups()
            if state == "detected":
                self._set_obstacle_monitor_state("detected", int(range_mm), int(threshold_mm), int(_clear_mm), device_name)
                self._set_firmware_obstacle_feedback(
                    f"MSG: 有障礙物，手臂已停止 | range={range_mm} mm | threshold={threshold_mm} mm | device={device_name}",
                    "error",
                )
                if motion_name is not None:
                    self.firmware_motion_var.set(f"Firmware motion {motion_name} stopped by obstacle")
                self._set_fw_command_feedback(
                    "FW safety: 有障礙物，手臂已停止",
                    f"FW echo range={range_mm} mm | threshold={threshold_mm} mm | device={device_name}",
                    tone="error",
                )
            else:
                self._set_obstacle_monitor_state("clear", int(range_mm), int(threshold_mm), int(_clear_mm), device_name)
                self._set_firmware_obstacle_feedback(
                    f"Obstacle monitor: clear | range={range_mm} mm | threshold={threshold_mm} mm | device={device_name}",
                    "success",
                )
            return

        home_match = CMD_HOME_PATTERN.match(line)
        if home_match is not None:
            duration_ms, clamp_us, fore_aft_us, gripper_lift_us, left_right_us = home_match.groups()
            status_text = "FW command result: HOME accepted"
            if duration_ms == "0":
                status_text += " | already at target"
            else:
                status_text += f" | duration {duration_ms} ms"
            self._set_fw_command_feedback(
                status_text,
                f"FW echo clamp={clamp_us} fore_aft={fore_aft_us} gripper_lift={gripper_lift_us} left_right={left_right_us} us",
                tone="success",
            )
            return

        pwm_match = CMD_PWM_PATTERN.match(line)
        if pwm_match is not None:
            requested_clamp, requested_fore_aft, requested_gripper_lift, requested_left_right, target_clamp, target_fore_aft, target_gripper_lift, target_left_right, duration_ms = pwm_match.groups()
            status_text = "FW command result: PWM accepted"
            if duration_ms == "0":
                status_text += " | no movement needed (already at target)"
            else:
                status_text += f" | duration {duration_ms} ms"
            self._set_fw_command_feedback(
                status_text,
                "FW echo "
                f"request={requested_clamp}/{requested_fore_aft}/{requested_gripper_lift}/{requested_left_right} us -> "
                f"target={target_clamp}/{target_fore_aft}/{target_gripper_lift}/{target_left_right} us",
                tone="success",
            )
            return

        pose_match = STATUS_POSE_PATTERN.match(line)
        if pose_match is not None:
            x_mm, y_mm, z_mm, yaw_deg, shoulder_deg, gripper_deg, clamp_us, fore_aft_us, gripper_lift_us, left_right_us = pose_match.groups()
            self.firmware_pose_var.set(
                "Firmware pose "
                f"x={x_mm} y={y_mm} z={z_mm} mm | "
                f"yaw={yaw_deg} shoulder={shoulder_deg} gripper={gripper_deg} deg | "
                f"PWM {clamp_us}/{fore_aft_us}/{gripper_lift_us}/{left_right_us} us"
            )
            return

        motion_match = STATUS_MOTION_PATTERN.match(line)
        if motion_match is not None:
            motion_name, remaining_ms, clamp_us, fore_aft_us, gripper_lift_us, left_right_us = motion_match.groups()
            self.firmware_motion_var.set(f"Firmware motion {motion_name} | remaining {remaining_ms} ms")
            if motion_name != "xyz":
                self.firmware_target_var.set(
                    "Firmware target PWM "
                    f"clamp={clamp_us} fore_aft={fore_aft_us} gripper_lift={gripper_lift_us} left_right={left_right_us} us"
                )
            return

        cartesian_match = STATUS_CARTESIAN_PATTERN.match(line)
        if cartesian_match is not None:
            x_mm, y_mm, z_mm, elbow_mode = cartesian_match.groups()
            self.firmware_target_var.set(
                f"Firmware Cartesian target x={x_mm} y={y_mm} z={z_mm} mm | elbow={elbow_mode}"
            )
            return

        imu_status_match = STATUS_IMU_PATTERN.match(line)
        if imu_status_match is not None:
            state, pitch_deg, roll_deg, gyro_mag_mdps, temp_dc = imu_status_match.groups()
            device_name = "mpu6050" if state != "disabled" else "sensor-not-ready"
            self._set_imu_monitor_state(
                state,
                int(pitch_deg),
                int(roll_deg),
                int(gyro_mag_mdps),
                int(temp_dc),
                device_name,
            )
            return

        obstacle_status_match = STATUS_OBSTACLE_PATTERN.match(line)
        if obstacle_status_match is not None:
            state, range_mm, threshold_mm, clear_mm, device_name = obstacle_status_match.groups()
            self._set_obstacle_monitor_state(state, int(range_mm), int(threshold_mm), int(clear_mm), device_name)
            if state == "detected":
                self._set_firmware_obstacle_feedback(
                    f"MSG: 有障礙物 | range={range_mm} mm | threshold={threshold_mm} mm | device={device_name}",
                    "error",
                )
            elif state == "clear":
                self._set_firmware_obstacle_feedback(
                    f"Obstacle monitor: clear | range={range_mm} mm | threshold={threshold_mm} mm | device={device_name}",
                    "success",
                )
            elif state == "unknown":
                self._set_firmware_obstacle_feedback(
                    f"Obstacle monitor: waiting for valid TOF range | last={range_mm} mm | device={device_name}",
                    "warning",
                )
            else:
                self._set_firmware_obstacle_feedback(
                    f"Obstacle monitor: unavailable | device={device_name}",
                    "neutral",
                )
            return

        xyz_queue_match = CMD_XYZ_CARTESIAN_PATTERN.match(line)
        if xyz_queue_match is not None:
            _start_x, _start_y, _start_z, target_x, target_y, target_z, elbow_mode, duration_ms = xyz_queue_match.groups()
            self.firmware_motion_var.set(f"Firmware motion xyz | remaining {duration_ms} ms")
            self.firmware_target_var.set(
                f"Firmware Cartesian target x={target_x} y={target_y} z={target_z} mm | elbow={elbow_mode}"
            )
            self._set_fw_command_feedback(
                f"FW command result: XYZ accepted | duration {duration_ms} ms",
                f"FW echo target x={target_x} y={target_y} z={target_z} mm | elbow={elbow_mode}",
                tone="success",
            )
            return

        xyz_fallback_match = CMD_XYZ_FALLBACK_PATTERN.match(line)
        if xyz_fallback_match is not None:
            target_x, target_y, target_z, elbow_mode, fore_aft_us, gripper_lift_us, left_right_us, duration_ms = xyz_fallback_match.groups()
            self._set_fw_command_feedback(
                f"FW command result: XYZ accepted with joint-space fallback | duration {duration_ms} ms",
                f"FW echo target x={target_x} y={target_y} z={target_z} mm | elbow={elbow_mode} | target PWM fore_aft={fore_aft_us} gripper_lift={gripper_lift_us} left_right={left_right_us} us",
                tone="success",
            )
            return

        xyz_reject_match = CMD_XYZ_REJECT_PATTERN.match(line)
        if xyz_reject_match is not None:
            target_x, target_y, target_z, reason = xyz_reject_match.groups()
            self._set_fw_command_feedback(
                "FW command result: XYZ rejected",
                f"FW echo target x={target_x} y={target_y} z={target_z} mm | reason={reason}",
                tone="error",
            )
            return

        usage_match = CMD_USAGE_PATTERN.match(line)
        if usage_match is not None:
            command_name, usage_text = usage_match.groups()
            self._set_fw_command_feedback(
                f"FW command result: {command_name} usage error",
                f"FW echo expected format: {usage_text}",
                tone="error",
            )
            return

        unknown_match = CMD_UNKNOWN_PATTERN.match(line)
        if unknown_match is not None:
            self._set_fw_command_feedback(
                "FW command result: unknown command",
                f"FW echo unknown input: {unknown_match.group(1)}",
                tone="error",
            )

    def send_home_command(self) -> None:
        self._workspace_drag_active = False
        self._workspace_drag_target = None
        self._cancel_workspace_preview_animation(clear_override=True)
        self._cancel_workspace_live_follow_send()
        self.load_startup_pwm()
        self.preview_mode_var.set("Local preview reset to the firmware HOME midpoint pose.")
        self._set_fw_command_feedback("FW command result: waiting for HOME acknowledgement", "FW echo: host sent HOME", tone="waiting")
        self.send_command("HOME")

    def send_xyz_command(self) -> None:
        self._workspace_drag_active = False
        self._workspace_drag_target = None
        self._cancel_workspace_preview_animation(clear_override=True)
        self._cancel_workspace_live_follow_send()
        input_issues = self._describe_current_xyz_input_issues()
        if input_issues:
            blocked_message = "XYZ send blocked: " + " | ".join(input_issues)
            self.preview_mode_var.set("Local preview blocked because one or more XYZ fields are invalid.")
            self.xyz_validation_var.set("Blocked: " + " | ".join(input_issues))
            self.append_log(f"[host] {blocked_message}")
            messagebox.showerror("Robot Arm Debug Tool", blocked_message)
            return

        target = self._current_target()
        if target is None:
            messagebox.showerror("Robot Arm Debug Tool", "XYZ values must be integers in millimeters.")
            return

        guide_violations = self._describe_target_guide_violations(target)
        if guide_violations:
            self.preview_mode_var.set("Local preview blocked because one or more XYZ fields are outside the guide range.")
            self._update_xyz_entry_states()
            messagebox.showerror("Robot Arm Debug Tool", "XYZ target is outside the conservative guide range:\n\n" + "\n".join(guide_violations))
            return

        self._use_live_preview()
        solution = self._solve_safe_ik(target[0], target[1], target[2])
        if solution is None:
            blocked_message = "XYZ send blocked: local IK cannot solve this target safely."
            self.preview_mode_var.set("Local preview blocked because the XYZ target is outside the local reachable workspace.")
            self.xyz_validation_var.set("Blocked: local IK cannot solve this XYZ target safely")
            self.append_log(f"[host] {blocked_message}")
            messagebox.showerror("Robot Arm Debug Tool", blocked_message)
            return

        left_right_pulse, fore_aft_pulse, gripper_lift_pulse = self._angles_to_pwm(*solution)
        self._set_pwm_values(self.clamp_var.get(), fore_aft_pulse, gripper_lift_pulse, left_right_pulse)
        self.preview_mode_var.set("Local preview shows the safe IK estimate for the queued XYZ move.")

        self._set_fw_command_feedback(
            "FW command result: waiting for XYZ acknowledgement",
            f"FW echo: host sent XYZ {target[0]} {target[1]} {target[2]}",
            tone="waiting",
        )
        self.send_command(f"XYZ {target[0]} {target[1]} {target[2]}")

    def send_pwm_command(self) -> None:
        self._workspace_drag_active = False
        self._workspace_drag_target = None
        self._cancel_workspace_preview_animation(clear_override=True)
        self._cancel_workspace_live_follow_send()
        self._use_live_preview()
        self.preview_mode_var.set("Local preview mirrors the PWM command about to be sent.")
        self._update_preview()
        self._set_fw_command_feedback(
            "FW command result: waiting for PWM acknowledgement",
            "FW echo: host sent PWM "
            f"{self.clamp_var.get()}/{self.fore_aft_var.get()}/{self.gripper_lift_var.get()}/{self.left_right_var.get()} us",
            tone="waiting",
        )
        self.send_command(
            f"PWM {self.clamp_var.get()} {self.fore_aft_var.get()} {self.gripper_lift_var.get()} {self.left_right_var.get()}"
        )

    def load_startup_pwm(self) -> None:
        self._workspace_drag_active = False
        self._workspace_drag_target = None
        self._cancel_workspace_preview_animation(clear_override=True)
        self._cancel_workspace_live_follow_send()
        self._use_live_preview()
        self._set_pwm_values(CLAMP_DEFAULT_PULSE, FORE_AFT_DEFAULT_PULSE, GRIPPER_LIFT_DEFAULT_PULSE, LEFT_RIGHT_DEFAULT_PULSE)
        self.preview_mode_var.set("Local preview follows the default midpoint pose.")

    def set_clamp_open(self) -> None:
        self.clamp_var.set(CLAMP_OPEN_PULSE)

    def set_clamp_close(self) -> None:
        self.clamp_var.set(CLAMP_CLOSE_PULSE)

    def _handle_close(self) -> None:
        self._cancel_workspace_preview_animation(clear_override=True)
        self._cancel_workspace_live_follow_send()
        self.disconnect_serial()
        self.destroy()


if __name__ == "__main__":
    app = RobotArmDebugGui()
    app.mainloop()
