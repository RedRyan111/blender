/* SPDX-FileCopyrightText: 2011-2022 Blender Foundation
 *
 * SPDX-License-Identifier: Apache-2.0 */

#pragma once
#include <corecrt_math.h>

CCL_NAMESPACE_BEGIN

/* Shape Node */
//Cycles CPU
ccl_device_noinline int svm_node_shape(KernelGlobals kg,
                                       ccl_private ShaderData *sd,
                                       ccl_private float *stack,
                                       uint value_stack_offset,
                                       uint parameters_stack_offsets,
                                       uint result_stack_offset, //maybe add uint result_stack_offset,
                                       int offset)
{

  //TODO: add rotation later
  uint sides_stack_offset, shape_type, scale_stack_offset, offset_stack_offset;
  svm_unpack_node_uchar4(parameters_stack_offsets, &sides_stack_offset, &offset_stack_offset, &scale_stack_offset, &shape_type);

  //uint4 defaults = read_node(kg, &offset);
  float3 value = stack_load_float3(stack, value_stack_offset);
  int sides = stack_load_float(stack, sides_stack_offset);
  float3 scale = stack_load_float3(stack, scale_stack_offset);
  float3 offset_vector = stack_load_float3(stack, offset_stack_offset);
  //float3 rotation = stack_load_float3(stack, rotation_stack_offset);

  //float3 b = stack_load_float3(stack, b_stack_offset);
  //float3 c = make_float3(0.0f, 0.0f, 0.0f);

  if (shape_type == NODE_SHAPE_POLYGON) {
      float3 position = make_float3(0.0f, 0.0f, 0.0f);
      position.x = (value.x - offset_vector.x) * scale.x;
      position.y = (value.y - offset_vector.y) * scale.y;

      float3 scaled_uvs = position * 2 * sides;
      float angle = atan2f(position.x, position.y);
      float slice = 3.14159 * 2.0 / sides;
      float length = safe_sqrtf(scaled_uvs.x * scaled_uvs.x + scaled_uvs.y * scaled_uvs.y);

      float result = (cosf(floorf((angle / slice) + .5) * slice - angle) * length) < 1;
      stack_store_float(stack, result_stack_offset, result);
  }
  else {
      float3 position = make_float3(0.0f, 0.0f, 0.0f);
      position.x = safe_powf(value.x - offset_vector.x, 2) / scale.x;
      position.y = safe_powf(value.y - offset_vector.y, 2) / scale.y;

      float summed = safe_sqrtf(position.x + position.y);
    
      float result = summed < 1;

    stack_store_float(stack, result_stack_offset, result);
  }
  return offset;
}

CCL_NAMESPACE_END