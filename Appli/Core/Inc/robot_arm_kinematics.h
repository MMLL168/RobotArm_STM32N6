#ifndef ROBOT_ARM_KINEMATICS_H
#define ROBOT_ARM_KINEMATICS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Spatial frame convention:
 * +X forward, +Y left, +Z up.
 * q0 = left_right base yaw about +Z.
 * q1 = fore_aft shoulder pitch in the radial-Z plane.
 * q2 = gripper_lift distal pitch relative to q1.
 * clamp opening is excluded from the spatial pose/Jacobian model.
 */

typedef enum
{
  ROBOT_ARM_IK_STATUS_OK = 0,
  ROBOT_ARM_IK_STATUS_INVALID_ARGUMENT,
  ROBOT_ARM_IK_STATUS_INVALID_GEOMETRY,
  ROBOT_ARM_IK_STATUS_OUT_OF_REACH,
  ROBOT_ARM_IK_STATUS_SINGULAR
} RobotArmIkStatus;

typedef enum
{
  ROBOT_ARM_ELBOW_MODE_DOWN = 0,
  ROBOT_ARM_ELBOW_MODE_UP
} RobotArmElbowMode;

typedef struct
{
  float x_mm;
  float y_mm;
  float z_mm;
} RobotArmVector3;

typedef struct
{
  float base_height_mm;
  float shoulder_length_mm;
  float forearm_length_mm;
  float tool_length_mm;
} RobotArmGeometry;

typedef struct
{
  float base_yaw_rad;
  float shoulder_pitch_rad;
  float gripper_lift_pitch_rad;
} RobotArmJointAngles;

typedef struct
{
  float x_mm;
  float y_mm;
  float z_mm;
  float radial_mm;
  float tool_pitch_rad;
} RobotArmPose;

typedef struct
{
  float minimum_rad;
  float maximum_rad;
} RobotArmJointLimit;

typedef struct
{
  RobotArmJointLimit base_yaw;
  RobotArmJointLimit shoulder_pitch;
  RobotArmJointLimit gripper_lift_pitch;
} RobotArmJointLimits;

typedef struct
{
  RobotArmVector3 minimum_position;
  RobotArmVector3 maximum_position;
  float minimum_horizontal_radius_mm;
  float maximum_horizontal_radius_mm;
} RobotArmWorkspaceBounds;

typedef struct
{
  float data[3][3];
} RobotArmJacobian;

bool RobotArmKinematics_Forward(const RobotArmGeometry *geometry,
                                const RobotArmJointAngles *joint_angles,
                                RobotArmPose *pose);

bool RobotArmKinematics_ComputeJacobian(const RobotArmGeometry *geometry,
                                        const RobotArmJointAngles *joint_angles,
                                        RobotArmJacobian *jacobian);

bool RobotArmKinematics_AreJointAnglesWithinLimits(const RobotArmJointAngles *joint_angles,
                                                   const RobotArmJointLimits *joint_limits);

bool RobotArmKinematics_EstimateWorkspaceBounds(const RobotArmGeometry *geometry,
                                                const RobotArmJointLimits *joint_limits,
                                                float minimum_z_mm,
                                                uint32_t samples_per_joint,
                                                RobotArmWorkspaceBounds *workspace_bounds);

RobotArmIkStatus RobotArmKinematics_InversePosition(const RobotArmGeometry *geometry,
                                                    const RobotArmVector3 *target_position,
                                                    RobotArmElbowMode elbow_mode,
                                                    RobotArmJointAngles *joint_solution);

RobotArmIkStatus RobotArmKinematics_SolveDifferentialIk(const RobotArmGeometry *geometry,
                                                        const RobotArmJointAngles *joint_angles,
                                                        const RobotArmVector3 *cartesian_delta_mm,
                                                        float damping,
                                                        RobotArmJointAngles *joint_delta);

const char *RobotArmKinematics_IkStatusString(RobotArmIkStatus status);

#ifdef __cplusplus
}
#endif

#endif