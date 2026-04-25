import math
import queue
import re
import threading
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

LEFT_RIGHT_MIN_PULSE = 1000
LEFT_RIGHT_MAX_PULSE = 2000
FORE_AFT_MIN_PULSE = 900
FORE_AFT_MAX_PULSE = 1800
GRIPPER_LIFT_MIN_PULSE = 1000
GRIPPER_LIFT_MAX_PULSE = 1500
CLAMP_OPEN_PULSE = 700
CLAMP_CLOSE_PULSE = 1600
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

PREVIEW_CANVAS_WIDTH = 560
PREVIEW_CANVAS_HEIGHT = 360

STATUS_POSE_PATTERN = re.compile(
    r"^\[status\] estimated_pose x=(-?\d+) y=(-?\d+) z=(-?\d+) mm q_deg yaw=(-?\d+) shoulder=(-?\d+) gripper=(-?\d+) pulses clamp=(\d+) fore_aft=(\d+) gripper_lift=(\d+) left_right=(\d+)$"
)
STATUS_MOTION_PATTERN = re.compile(
    r"^\[status\] motion=([a-z_]+) remaining=(\d+) ms target_pulses clamp=(\d+) fore_aft=(\d+) gripper_lift=(\d+) left_right=(\d+)$"
)
STATUS_CARTESIAN_PATTERN = re.compile(
    r"^\[status\] cartesian target_xyz x=(-?\d+) y=(-?\d+) z=(-?\d+) mm elbow=([a-z]+)$"
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
        self.geometry("1280x820")
        self.minsize(900, 620)

        self.workspace_bounds = self._estimate_workspace_bounds()
        self.preview_bounds = self._estimate_preview_bounds()
        default_x_mm = self._guide_axis_midpoint("x")
        default_y_mm = self._guide_axis_midpoint("y")
        default_z_mm = self._guide_axis_midpoint("z")

        self.port_var = tk.StringVar()
        self.connection_var = tk.StringVar(value="Disconnected")
        self.connect_button_var = tk.StringVar(value="Connect")

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
            value="Orange shoulder housing = Fore front/back joint. Blue elbow housing = Gripper up/down joint. Pink marker = XYZ target."
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

        self.serial_port = None
        self.reader_stop_event = threading.Event()
        self.reader_thread = None
        self.status_poll_after_id = None
        self.log_queue = queue.Queue()
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
        self.command_layout_mode = None
        self._last_preview_canvas_size = (0, 0)
        self._suspend_preview_updates = False
        self._initial_preview_joint_angles = self._create_initial_preview_joint_angles()

        self._build_ui()
        self._install_variable_traces()
        self._update_xyz_entry_states()
        self._refresh_xyz_validation_state()
        self.refresh_ports()
        self._update_preview()
        self.after(100, self._poll_log_queue)
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
        self._add_scale(self.pwm_frame, "Left/Right", self.left_right_var, LEFT_RIGHT_MIN_PULSE, LEFT_RIGHT_MAX_PULSE, 3)
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
        ttk.Label(self.preview_frame, textvariable=self.preview_legend_var, wraplength=492, justify="left", font=("Segoe UI", 9)).grid(
            row=1, column=0, sticky="ew", padx=8, pady=(0, 2)
        )
        ttk.Label(self.preview_frame, textvariable=self.preview_mode_var, wraplength=492, justify="left", font=("Segoe UI", 9)).grid(
            row=2, column=0, sticky="ew", padx=8, pady=(0, 2)
        )
        ttk.Label(self.preview_frame, textvariable=self.pose_summary_var, wraplength=492, justify="left", font=("Segoe UI", 9)).grid(
            row=3, column=0, sticky="ew", padx=8, pady=(0, 2)
        )
        ttk.Label(self.preview_frame, textvariable=self.target_summary_var, wraplength=492, justify="left", font=("Segoe UI", 9)).grid(
            row=4, column=0, sticky="ew", padx=8, pady=(0, 8)
        )
        ttk.Separator(self.preview_frame, orient="horizontal").grid(row=5, column=0, sticky="ew", padx=8, pady=(0, 6))
        ttk.Label(self.preview_frame, textvariable=self.firmware_motion_var, wraplength=492, justify="left", font=("Segoe UI", 9, "bold")).grid(
            row=6, column=0, sticky="ew", padx=8, pady=(0, 2)
        )
        ttk.Label(self.preview_frame, textvariable=self.firmware_pose_var, wraplength=492, justify="left", font=("Segoe UI", 9)).grid(
            row=7, column=0, sticky="ew", padx=8, pady=(0, 2)
        )
        ttk.Label(self.preview_frame, textvariable=self.firmware_target_var, wraplength=492, justify="left", font=("Segoe UI", 9)).grid(
            row=8, column=0, sticky="ew", padx=8, pady=(0, 8)
        )

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
        ttk.Button(bottom_frame, text="Clear Log", command=self.clear_log).pack(side="left", padx=8)

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

        self.preview_mode_var.set("Local preview follows manual PWM sliders.")
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

    def _update_preview(self) -> None:
        if self._initial_preview_joint_angles is not None:
            yaw_rad, shoulder_rad, gripper_rad = self._initial_preview_joint_angles
        else:
            yaw_rad, shoulder_rad, gripper_rad = self._current_joint_angles()
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
        canvas.create_oval(content_left - 80, content_top - 70, content_left + 250, content_top + 210, fill="#122b49", outline="")
        canvas.create_oval(content_left + 250, content_top + 30, content_left + 620, content_top + 320, fill="#1a203d", outline="")
        canvas.create_rectangle(0, content_bottom - 52, width, content_bottom, fill="#081723", outline="")

        side_left = content_left + 14
        side_top = content_top + 26
        side_right = content_left + 370
        side_bottom = content_top + PREVIEW_CANVAS_HEIGHT - 30
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

        radar_left = content_left + 392
        radar_top = content_top + 32
        radar_right = content_left + PREVIEW_CANVAS_WIDTH - 20
        radar_bottom = content_top + PREVIEW_CANVAS_HEIGHT - 34
        radar_width = radar_right - radar_left
        radar_height = radar_bottom - radar_top
        radar_radius = int(min((radar_width - 34) * 0.5, (radar_height - 150) * 0.5, 64))
        radar_center_x = (radar_left + radar_right) * 0.5
        radar_center_y = radar_top + radar_radius + 44
        radar_text_width = radar_width - 28
        canvas.create_rectangle(radar_left, radar_top, radar_right, radar_bottom, fill="#0d1628", outline="#233a57", width=2)
        canvas.create_text(radar_left + 14, radar_top + 18, anchor="w", text="Yaw / Reach", fill="#d8f3ff", font=("Segoe UI", 10, "bold"))

        canvas.create_oval(radar_center_x - radar_radius, radar_center_y - radar_radius, radar_center_x + radar_radius, radar_center_y + radar_radius, outline="#29517a", width=2)
        canvas.create_oval(radar_center_x - (radar_radius * 0.64), radar_center_y - (radar_radius * 0.64), radar_center_x + (radar_radius * 0.64), radar_center_y + (radar_radius * 0.64), outline="#20415f")
        canvas.create_oval(radar_center_x - (radar_radius * 0.28), radar_center_y - (radar_radius * 0.28), radar_center_x + (radar_radius * 0.28), radar_center_y + (radar_radius * 0.28), outline="#20415f")
        canvas.create_line(radar_center_x - radar_radius, radar_center_y, radar_center_x + radar_radius, radar_center_y, fill="#20415f")
        canvas.create_line(radar_center_x, radar_center_y - radar_radius, radar_center_x, radar_center_y + radar_radius, fill="#20415f")

        radar_scale = radar_radius / 250.0
        actual_radar_x = radar_center_x + (pose["x"] * radar_scale)
        actual_radar_y = radar_center_y - (pose["y"] * radar_scale)
        canvas.create_line(radar_center_x, radar_center_y, actual_radar_x, actual_radar_y, fill="#6be0ff", width=5, capstyle=tk.ROUND)
        canvas.create_oval(actual_radar_x - 8, actual_radar_y - 8, actual_radar_x + 8, actual_radar_y + 8, fill="#ffe082", outline="#fff8d0")

        if target is not None:
            target_radar_x = radar_center_x + (target[0] * radar_scale)
            target_radar_y = radar_center_y - (target[1] * radar_scale)
            canvas.create_oval(target_radar_x - 9, target_radar_y - 9, target_radar_x + 9, target_radar_y + 9, outline="#ff5b8f", width=3)
            canvas.create_line(target_radar_x - 13, target_radar_y, target_radar_x + 13, target_radar_y, fill="#ff5b8f", width=2)
            canvas.create_line(target_radar_x, target_radar_y - 13, target_radar_x, target_radar_y + 13, fill="#ff5b8f", width=2)

        canvas.create_text(
            radar_left + 14,
            radar_bottom - 98,
            anchor="nw",
            text=f"Yaw {math.degrees(yaw_rad):.0f} deg",
            fill="#a9dcff",
            font=("Segoe UI", 10, "bold"),
            width=radar_text_width,
        )
        canvas.create_text(
            radar_left + 14,
            radar_bottom - 72,
            anchor="nw",
            text=f"Clamp {self.clamp_var.get()} us | Reach {pose['radial']:.0f} mm",
            fill="#c8dcf4",
            font=("Segoe UI", 9),
            width=radar_text_width,
        )

    def refresh_ports(self) -> None:
        ports = sorted(port.device for port in list_ports.comports())
        self.port_combo["values"] = ports
        if ports and self.port_var.get() not in ports:
            self.port_var.set(ports[0])
        if not ports:
            self.port_var.set("")

    def toggle_connection(self) -> None:
        if self.serial_port is None:
            self.connect_serial()
        else:
            self.disconnect_serial()

    def connect_serial(self) -> None:
        port = self.port_var.get().strip()
        if not port:
            messagebox.showerror("Robot Arm Debug Tool", "Select a serial port first.")
            return

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
        self._set_fw_command_feedback("FW command result: connected, waiting for next command", "FW echo: no command acknowledgement yet", tone="neutral")
        self.append_log(f"[host] connected to {port}")
        self._schedule_status_poll(initial_delay_ms=150)

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

        self.after(100, self._poll_log_queue)

    def append_log(self, line: str) -> None:
        self._handle_firmware_line(line)
        self.log_text.configure(state="normal")
        self.log_text.insert("end", line + "\n")
        self.log_text.see("end")
        self.log_text.configure(state="disabled")

    def clear_log(self) -> None:
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

    def _schedule_status_poll(self, initial_delay_ms: int = 500) -> None:
        self._cancel_status_poll()
        if self.serial_port is None:
            return

        self.status_poll_after_id = self.after(initial_delay_ms, self._request_status_poll)

    def _request_status_poll(self) -> None:
        self.status_poll_after_id = None
        if self.serial_port is None:
            return

        if self._write_serial_command("STATUS", log_command=False):
            self._schedule_status_poll(initial_delay_ms=500)

    def _handle_firmware_line(self, line: str) -> None:
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
        self.load_startup_pwm()
        self.preview_mode_var.set("Local preview reset to the firmware HOME midpoint pose.")
        self._set_fw_command_feedback("FW command result: waiting for HOME acknowledgement", "FW echo: host sent HOME", tone="waiting")
        self.send_command("HOME")

    def send_xyz_command(self) -> None:
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
        self._use_live_preview()
        self._set_pwm_values(CLAMP_DEFAULT_PULSE, FORE_AFT_DEFAULT_PULSE, GRIPPER_LIFT_DEFAULT_PULSE, LEFT_RIGHT_DEFAULT_PULSE)
        self.preview_mode_var.set("Local preview follows the default midpoint pose.")

    def set_clamp_open(self) -> None:
        self.clamp_var.set(CLAMP_OPEN_PULSE)

    def set_clamp_close(self) -> None:
        self.clamp_var.set(CLAMP_CLOSE_PULSE)

    def _handle_close(self) -> None:
        self.disconnect_serial()
        self.destroy()


if __name__ == "__main__":
    app = RobotArmDebugGui()
    app.mainloop()