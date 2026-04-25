#include "robot_arm_kinematics.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#define ROBOT_ARM_EPSILON 1.0e-6f

static bool RobotArmKinematics_IsGeometryValid(const RobotArmGeometry *geometry)
{
  float distal_length;

  if (geometry == NULL)
  {
    return false;
  }

  if (geometry->shoulder_length_mm <= 0.0f)
  {
    return false;
  }

  if (geometry->forearm_length_mm < 0.0f || geometry->tool_length_mm < 0.0f)
  {
    return false;
  }

  distal_length = geometry->forearm_length_mm + geometry->tool_length_mm;
  return distal_length > 0.0f;
}

static float RobotArmKinematics_Clamp(float value, float minimum, float maximum)
{
  if (value < minimum)
  {
    return minimum;
  }

  if (value > maximum)
  {
    return maximum;
  }

  return value;
}

static bool RobotArmKinematics_IsJointLimitValid(const RobotArmJointLimit *joint_limit)
{
  if (joint_limit == NULL)
  {
    return false;
  }

  return joint_limit->minimum_rad <= joint_limit->maximum_rad;
}

static float RobotArmKinematics_Lerp(float minimum, float maximum, float ratio)
{
  return minimum + ((maximum - minimum) * ratio);
}

static bool RobotArmKinematics_Invert3x3(const float input[3][3], float output[3][3], float *determinant_out)
{
  float cofactor00;
  float cofactor01;
  float cofactor02;
  float cofactor10;
  float cofactor11;
  float cofactor12;
  float cofactor20;
  float cofactor21;
  float cofactor22;
  float determinant;
  float inverse_determinant;

  cofactor00 = (input[1][1] * input[2][2]) - (input[1][2] * input[2][1]);
  cofactor01 = -((input[1][0] * input[2][2]) - (input[1][2] * input[2][0]));
  cofactor02 = (input[1][0] * input[2][1]) - (input[1][1] * input[2][0]);
  cofactor10 = -((input[0][1] * input[2][2]) - (input[0][2] * input[2][1]));
  cofactor11 = (input[0][0] * input[2][2]) - (input[0][2] * input[2][0]);
  cofactor12 = -((input[0][0] * input[2][1]) - (input[0][1] * input[2][0]));
  cofactor20 = (input[0][1] * input[1][2]) - (input[0][2] * input[1][1]);
  cofactor21 = -((input[0][0] * input[1][2]) - (input[0][2] * input[1][0]));
  cofactor22 = (input[0][0] * input[1][1]) - (input[0][1] * input[1][0]);

  determinant = (input[0][0] * cofactor00) + (input[0][1] * cofactor01) + (input[0][2] * cofactor02);
  if (determinant_out != NULL)
  {
    *determinant_out = determinant;
  }

  if (fabsf(determinant) < ROBOT_ARM_EPSILON)
  {
    return false;
  }

  inverse_determinant = 1.0f / determinant;

  output[0][0] = cofactor00 * inverse_determinant;
  output[0][1] = cofactor10 * inverse_determinant;
  output[0][2] = cofactor20 * inverse_determinant;
  output[1][0] = cofactor01 * inverse_determinant;
  output[1][1] = cofactor11 * inverse_determinant;
  output[1][2] = cofactor21 * inverse_determinant;
  output[2][0] = cofactor02 * inverse_determinant;
  output[2][1] = cofactor12 * inverse_determinant;
  output[2][2] = cofactor22 * inverse_determinant;

  return true;
}

bool RobotArmKinematics_Forward(const RobotArmGeometry *geometry,
                                const RobotArmJointAngles *joint_angles,
                                RobotArmPose *pose)
{
  float distal_length;
  float summed_pitch;
  float radial_mm;

  if (!RobotArmKinematics_IsGeometryValid(geometry) || joint_angles == NULL || pose == NULL)
  {
    return false;
  }

  distal_length = geometry->forearm_length_mm + geometry->tool_length_mm;
  summed_pitch = joint_angles->shoulder_pitch_rad + joint_angles->gripper_lift_pitch_rad;
  radial_mm = (geometry->shoulder_length_mm * cosf(joint_angles->shoulder_pitch_rad)) +
              (distal_length * cosf(summed_pitch));

  pose->x_mm = radial_mm * cosf(joint_angles->base_yaw_rad);
  pose->y_mm = radial_mm * sinf(joint_angles->base_yaw_rad);
  pose->z_mm = geometry->base_height_mm +
               (geometry->shoulder_length_mm * sinf(joint_angles->shoulder_pitch_rad)) +
               (distal_length * sinf(summed_pitch));
  pose->radial_mm = radial_mm;
  pose->tool_pitch_rad = summed_pitch;

  return true;
}

bool RobotArmKinematics_ComputeJacobian(const RobotArmGeometry *geometry,
                                        const RobotArmJointAngles *joint_angles,
                                        RobotArmJacobian *jacobian)
{
  float distal_length;
  float summed_pitch;
  float radial_mm;
  float radial_q1;
  float radial_q2;
  float z_q1;
  float z_q2;
  float cos_yaw;
  float sin_yaw;

  if (!RobotArmKinematics_IsGeometryValid(geometry) || joint_angles == NULL || jacobian == NULL)
  {
    return false;
  }

  distal_length = geometry->forearm_length_mm + geometry->tool_length_mm;
  summed_pitch = joint_angles->shoulder_pitch_rad + joint_angles->gripper_lift_pitch_rad;
  cos_yaw = cosf(joint_angles->base_yaw_rad);
  sin_yaw = sinf(joint_angles->base_yaw_rad);

  radial_mm = (geometry->shoulder_length_mm * cosf(joint_angles->shoulder_pitch_rad)) +
              (distal_length * cosf(summed_pitch));
  radial_q1 = -(geometry->shoulder_length_mm * sinf(joint_angles->shoulder_pitch_rad)) -
              (distal_length * sinf(summed_pitch));
  radial_q2 = -(distal_length * sinf(summed_pitch));
  z_q1 = (geometry->shoulder_length_mm * cosf(joint_angles->shoulder_pitch_rad)) +
         (distal_length * cosf(summed_pitch));
  z_q2 = distal_length * cosf(summed_pitch);

  jacobian->data[0][0] = -radial_mm * sin_yaw;
  jacobian->data[1][0] = radial_mm * cos_yaw;
  jacobian->data[2][0] = 0.0f;

  jacobian->data[0][1] = cos_yaw * radial_q1;
  jacobian->data[1][1] = sin_yaw * radial_q1;
  jacobian->data[2][1] = z_q1;

  jacobian->data[0][2] = cos_yaw * radial_q2;
  jacobian->data[1][2] = sin_yaw * radial_q2;
  jacobian->data[2][2] = z_q2;

  return true;
}

bool RobotArmKinematics_AreJointAnglesWithinLimits(const RobotArmJointAngles *joint_angles,
                                                   const RobotArmJointLimits *joint_limits)
{
  if (joint_angles == NULL || joint_limits == NULL)
  {
    return false;
  }

  if (!RobotArmKinematics_IsJointLimitValid(&joint_limits->base_yaw) ||
      !RobotArmKinematics_IsJointLimitValid(&joint_limits->shoulder_pitch) ||
      !RobotArmKinematics_IsJointLimitValid(&joint_limits->gripper_lift_pitch))
  {
    return false;
  }

  return joint_angles->base_yaw_rad >= joint_limits->base_yaw.minimum_rad &&
         joint_angles->base_yaw_rad <= joint_limits->base_yaw.maximum_rad &&
         joint_angles->shoulder_pitch_rad >= joint_limits->shoulder_pitch.minimum_rad &&
         joint_angles->shoulder_pitch_rad <= joint_limits->shoulder_pitch.maximum_rad &&
         joint_angles->gripper_lift_pitch_rad >= joint_limits->gripper_lift_pitch.minimum_rad &&
         joint_angles->gripper_lift_pitch_rad <= joint_limits->gripper_lift_pitch.maximum_rad;
}

bool RobotArmKinematics_EstimateWorkspaceBounds(const RobotArmGeometry *geometry,
                                                const RobotArmJointLimits *joint_limits,
                                                float minimum_z_mm,
                                                uint32_t samples_per_joint,
                                                RobotArmWorkspaceBounds *workspace_bounds)
{
  RobotArmJointAngles sample_joint_angles;
  RobotArmPose sample_pose;
  float horizontal_radius_mm;
  float ratio_yaw;
  float ratio_shoulder;
  float ratio_gripper;
  uint32_t yaw_index;
  uint32_t shoulder_index;
  uint32_t gripper_index;
  bool has_workspace_sample = false;

  if (!RobotArmKinematics_IsGeometryValid(geometry) ||
      joint_limits == NULL ||
      workspace_bounds == NULL ||
      samples_per_joint < 2U)
  {
    return false;
  }

  if (!RobotArmKinematics_IsJointLimitValid(&joint_limits->base_yaw) ||
      !RobotArmKinematics_IsJointLimitValid(&joint_limits->shoulder_pitch) ||
      !RobotArmKinematics_IsJointLimitValid(&joint_limits->gripper_lift_pitch))
  {
    return false;
  }

  for (yaw_index = 0U; yaw_index < samples_per_joint; ++yaw_index)
  {
    ratio_yaw = (float)yaw_index / (float)(samples_per_joint - 1U);
    sample_joint_angles.base_yaw_rad = RobotArmKinematics_Lerp(joint_limits->base_yaw.minimum_rad,
                                                               joint_limits->base_yaw.maximum_rad,
                                                               ratio_yaw);

    for (shoulder_index = 0U; shoulder_index < samples_per_joint; ++shoulder_index)
    {
      ratio_shoulder = (float)shoulder_index / (float)(samples_per_joint - 1U);
      sample_joint_angles.shoulder_pitch_rad = RobotArmKinematics_Lerp(joint_limits->shoulder_pitch.minimum_rad,
                                                                       joint_limits->shoulder_pitch.maximum_rad,
                                                                       ratio_shoulder);

      for (gripper_index = 0U; gripper_index < samples_per_joint; ++gripper_index)
      {
        ratio_gripper = (float)gripper_index / (float)(samples_per_joint - 1U);
        sample_joint_angles.gripper_lift_pitch_rad = RobotArmKinematics_Lerp(joint_limits->gripper_lift_pitch.minimum_rad,
                                                                             joint_limits->gripper_lift_pitch.maximum_rad,
                                                                             ratio_gripper);

        if (!RobotArmKinematics_Forward(geometry, &sample_joint_angles, &sample_pose))
        {
          return false;
        }

        if (sample_pose.z_mm < minimum_z_mm)
        {
          continue;
        }

        horizontal_radius_mm = sqrtf((sample_pose.x_mm * sample_pose.x_mm) +
                                     (sample_pose.y_mm * sample_pose.y_mm));

        if (!has_workspace_sample)
        {
          workspace_bounds->minimum_position.x_mm = sample_pose.x_mm;
          workspace_bounds->minimum_position.y_mm = sample_pose.y_mm;
          workspace_bounds->minimum_position.z_mm = sample_pose.z_mm;
          workspace_bounds->maximum_position.x_mm = sample_pose.x_mm;
          workspace_bounds->maximum_position.y_mm = sample_pose.y_mm;
          workspace_bounds->maximum_position.z_mm = sample_pose.z_mm;
          workspace_bounds->minimum_horizontal_radius_mm = horizontal_radius_mm;
          workspace_bounds->maximum_horizontal_radius_mm = horizontal_radius_mm;
          has_workspace_sample = true;
          continue;
        }

        if (sample_pose.x_mm < workspace_bounds->minimum_position.x_mm)
        {
          workspace_bounds->minimum_position.x_mm = sample_pose.x_mm;
        }

        if (sample_pose.y_mm < workspace_bounds->minimum_position.y_mm)
        {
          workspace_bounds->minimum_position.y_mm = sample_pose.y_mm;
        }

        if (sample_pose.z_mm < workspace_bounds->minimum_position.z_mm)
        {
          workspace_bounds->minimum_position.z_mm = sample_pose.z_mm;
        }

        if (sample_pose.x_mm > workspace_bounds->maximum_position.x_mm)
        {
          workspace_bounds->maximum_position.x_mm = sample_pose.x_mm;
        }

        if (sample_pose.y_mm > workspace_bounds->maximum_position.y_mm)
        {
          workspace_bounds->maximum_position.y_mm = sample_pose.y_mm;
        }

        if (sample_pose.z_mm > workspace_bounds->maximum_position.z_mm)
        {
          workspace_bounds->maximum_position.z_mm = sample_pose.z_mm;
        }

        if (horizontal_radius_mm < workspace_bounds->minimum_horizontal_radius_mm)
        {
          workspace_bounds->minimum_horizontal_radius_mm = horizontal_radius_mm;
        }

        if (horizontal_radius_mm > workspace_bounds->maximum_horizontal_radius_mm)
        {
          workspace_bounds->maximum_horizontal_radius_mm = horizontal_radius_mm;
        }
      }
    }
  }

  return has_workspace_sample;
}

RobotArmIkStatus RobotArmKinematics_InversePosition(const RobotArmGeometry *geometry,
                                                    const RobotArmVector3 *target_position,
                                                    RobotArmElbowMode elbow_mode,
                                                    RobotArmJointAngles *joint_solution)
{
  float distal_length;
  float radial_mm;
  float z_from_shoulder_mm;
  float cos_elbow;
  float sin_elbow_sq;
  float sin_elbow;

  if (geometry == NULL || target_position == NULL || joint_solution == NULL)
  {
    return ROBOT_ARM_IK_STATUS_INVALID_ARGUMENT;
  }

  if (!RobotArmKinematics_IsGeometryValid(geometry))
  {
    return ROBOT_ARM_IK_STATUS_INVALID_GEOMETRY;
  }

  distal_length = geometry->forearm_length_mm + geometry->tool_length_mm;
  radial_mm = sqrtf((target_position->x_mm * target_position->x_mm) +
                    (target_position->y_mm * target_position->y_mm));
  z_from_shoulder_mm = target_position->z_mm - geometry->base_height_mm;

  cos_elbow = ((radial_mm * radial_mm) + (z_from_shoulder_mm * z_from_shoulder_mm) -
               (geometry->shoulder_length_mm * geometry->shoulder_length_mm) -
               (distal_length * distal_length)) /
              (2.0f * geometry->shoulder_length_mm * distal_length);

  if (cos_elbow < (-1.0f - ROBOT_ARM_EPSILON) || cos_elbow > (1.0f + ROBOT_ARM_EPSILON))
  {
    return ROBOT_ARM_IK_STATUS_OUT_OF_REACH;
  }

  cos_elbow = RobotArmKinematics_Clamp(cos_elbow, -1.0f, 1.0f);
  sin_elbow_sq = 1.0f - (cos_elbow * cos_elbow);
  if (sin_elbow_sq < -ROBOT_ARM_EPSILON)
  {
    return ROBOT_ARM_IK_STATUS_OUT_OF_REACH;
  }

  if (sin_elbow_sq < 0.0f)
  {
    sin_elbow_sq = 0.0f;
  }

  sin_elbow = sqrtf(sin_elbow_sq);
  if (elbow_mode == ROBOT_ARM_ELBOW_MODE_DOWN)
  {
    sin_elbow = -sin_elbow;
  }

  joint_solution->base_yaw_rad = atan2f(target_position->y_mm, target_position->x_mm);
  joint_solution->gripper_lift_pitch_rad = atan2f(sin_elbow, cos_elbow);
  joint_solution->shoulder_pitch_rad = atan2f(z_from_shoulder_mm, radial_mm) -
                                       atan2f(distal_length * sin_elbow,
                                              geometry->shoulder_length_mm + (distal_length * cos_elbow));

  return ROBOT_ARM_IK_STATUS_OK;
}

RobotArmIkStatus RobotArmKinematics_SolveDifferentialIk(const RobotArmGeometry *geometry,
                                                        const RobotArmJointAngles *joint_angles,
                                                        const RobotArmVector3 *cartesian_delta_mm,
                                                        float damping,
                                                        RobotArmJointAngles *joint_delta)
{
  RobotArmJacobian jacobian;
  float jjt[3][3] = {0};
  float jjt_inverse[3][3];
  float intermediate[3] = {0};
  float damping_sq;
  uint32_t row;
  uint32_t column;
  uint32_t index;

  if (geometry == NULL || joint_angles == NULL || cartesian_delta_mm == NULL || joint_delta == NULL || damping < 0.0f)
  {
    return ROBOT_ARM_IK_STATUS_INVALID_ARGUMENT;
  }

  if (!RobotArmKinematics_IsGeometryValid(geometry))
  {
    return ROBOT_ARM_IK_STATUS_INVALID_GEOMETRY;
  }

  if (!RobotArmKinematics_ComputeJacobian(geometry, joint_angles, &jacobian))
  {
    return ROBOT_ARM_IK_STATUS_INVALID_GEOMETRY;
  }

  for (row = 0U; row < 3U; ++row)
  {
    for (column = 0U; column < 3U; ++column)
    {
      for (index = 0U; index < 3U; ++index)
      {
        jjt[row][column] += jacobian.data[row][index] * jacobian.data[column][index];
      }
    }
  }

  damping_sq = damping * damping;
  jjt[0][0] += damping_sq;
  jjt[1][1] += damping_sq;
  jjt[2][2] += damping_sq;

  if (!RobotArmKinematics_Invert3x3(jjt, jjt_inverse, NULL))
  {
    return ROBOT_ARM_IK_STATUS_SINGULAR;
  }

  intermediate[0] = (jjt_inverse[0][0] * cartesian_delta_mm->x_mm) +
                    (jjt_inverse[0][1] * cartesian_delta_mm->y_mm) +
                    (jjt_inverse[0][2] * cartesian_delta_mm->z_mm);
  intermediate[1] = (jjt_inverse[1][0] * cartesian_delta_mm->x_mm) +
                    (jjt_inverse[1][1] * cartesian_delta_mm->y_mm) +
                    (jjt_inverse[1][2] * cartesian_delta_mm->z_mm);
  intermediate[2] = (jjt_inverse[2][0] * cartesian_delta_mm->x_mm) +
                    (jjt_inverse[2][1] * cartesian_delta_mm->y_mm) +
                    (jjt_inverse[2][2] * cartesian_delta_mm->z_mm);

  joint_delta->base_yaw_rad = (jacobian.data[0][0] * intermediate[0]) +
                              (jacobian.data[1][0] * intermediate[1]) +
                              (jacobian.data[2][0] * intermediate[2]);
  joint_delta->shoulder_pitch_rad = (jacobian.data[0][1] * intermediate[0]) +
                                    (jacobian.data[1][1] * intermediate[1]) +
                                    (jacobian.data[2][1] * intermediate[2]);
  joint_delta->gripper_lift_pitch_rad = (jacobian.data[0][2] * intermediate[0]) +
                                        (jacobian.data[1][2] * intermediate[1]) +
                                        (jacobian.data[2][2] * intermediate[2]);

  return ROBOT_ARM_IK_STATUS_OK;
}

const char *RobotArmKinematics_IkStatusString(RobotArmIkStatus status)
{
  switch (status)
  {
    case ROBOT_ARM_IK_STATUS_OK:
      return "ok";

    case ROBOT_ARM_IK_STATUS_INVALID_ARGUMENT:
      return "invalid-argument";

    case ROBOT_ARM_IK_STATUS_INVALID_GEOMETRY:
      return "invalid-geometry";

    case ROBOT_ARM_IK_STATUS_OUT_OF_REACH:
      return "out-of-reach";

    case ROBOT_ARM_IK_STATUS_SINGULAR:
      return "singular";

    default:
      return "unknown";
  }
}