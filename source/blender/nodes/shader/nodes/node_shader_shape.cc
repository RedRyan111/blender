/* SPDX-FileCopyrightText: 2005 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup shdnodes
 */

#include "node_shader_util.hh"
#include "FN_multi_function_builder.hh"

#include "NOD_multi_function.hh"

#include "UI_interface.hh"
#include "UI_resources.hh"

namespace blender::nodes::node_shader_shape_cc {

static void sh_node_shape_declare(NodeDeclarationBuilder &b)
{
  b.is_function_node();
  b.add_input<decl::Vector>("Vector").min(-10000.0f).max(10000.0f).implicit_field(implicit_field_inputs::position);
  b.add_input<decl::Int>("Sides").default_value(4.0f).min(0);
  b.add_input<decl::Vector>("Offset")      
      .default_value({0.0f, 0.0f, 0.0f})
      .description("The vector to control offset");
  b.add_input<decl::Vector>("Scale")
      .default_value({1.0f, 1.0f, 1.0f})
      .description("The vector to control scale");
  b.add_output<decl::Float>("Result");
}

static void node_shader_buts_shape(uiLayout *layout, bContext * /*C*/, PointerRNA *ptr)
{
  uiItemR(layout, ptr, "shape_type", UI_ITEM_R_SPLIT_EMPTY_NAME, "", ICON_NONE);
}

static void node_shader_init_shape(bNodeTree * /*ntree*/, bNode *node)
{
  node->custom1 = NODE_SHAPE_POLYGON; /* shape type */
}

static int gpu_shader_shape(GPUMaterial *mat,
                            bNode *node,
                            bNodeExecData * /*execdata*/,
                            GPUNodeStack *in,
                            GPUNodeStack *out)
{
  return (node->custom1 == NODE_SHAPE_POLYGON) ?
             GPU_stack_link(mat, node, "shape_polygon", in, out) :
             GPU_stack_link(mat, node, "shape_circle", in, out);
}
//Used by eevee?
static void sh_node_shape_build_multi_function(NodeMultiFunctionBuilder &builder)
{
  static auto polygon_fn = mf::build::SI4_SO<float3, int, float3, float3, float>(
      "Shape (Polygon)",
      [](const float3 &uv_vec, int sides, float3 offset, float3 scale) { 
        //return 1;
        
        float3 position = uv_vec;
        position.x = (uv_vec[0] - offset[0] + .5) * scale.x;
        position.y = (uv_vec[1] - offset[1] + .5) * scale.y;
        return (abs(position[0]) < 1) * (abs(position[1]) < 1);
        /*float angle = std::atan2(position.x, position.y);

        float slice = M_PI * 2.0 / sides; // maybe pi_v?? //M_PI

        float3 temp = math::square(position);
        float length = math::sqrt(temp.x + temp.y);

        float ret = std::cos(std::floor(.5 + angle/slice) * slice - angle) * length;

        return ret < 1; */
        
        });
  static auto circle_fn = mf::build::SI4_SO<float3, int, float3, float3, float>(
      "Shape (Circle)",
      [](const float3 &uv_vec, int sides, float3 offset, float3 scale) { 

      /*float3 position = in;
      position.x = (in.x - offset.x + .5) * scale.x;
      position.y = (in.y - offset.y + .5) * scale.y;

      float3 abs_position = {fabs(position.x), fabs(position.y), fabs(position.z)};

      float3 position_squared = math::square(abs_position);
      float summed = math::sqrt(position.x + position.y);
      return summed < 1;*/
      return .1;

      });

  int shape_type = builder.node().custom1;
  if (shape_type == NODE_SHAPE_POLYGON) {
    builder.set_matching_fn(polygon_fn);
  }
  else {
    builder.set_matching_fn(circle_fn);
  }
}
/*
NODE_SHADER_MATERIALX_BEGIN
#ifdef WITH_MATERIALX
{
  auto type = node_->custom1;
  NodeItem value = get_input_value("Value", NodeItem::Type::Float);
  NodeItem min = get_input_value("Min", NodeItem::Type::Float);
  NodeItem max = get_input_value("Max", NodeItem::Type::Float);

  NodeItem res = empty();
  if (type == NODE_CLAMP_RANGE) {
    res = min.if_else(
        NodeItem::CompareOp::Less, max, value.clamp(min, max), value.clamp(max, min));
  }
  else {
    res = value.clamp(min, max);
  }
  return res;
}
#endif
NODE_SHADER_MATERIALX_END
*/

}  // namespace blender::nodes::node_shader_shape_cc

void register_node_type_sh_shape() //add to node_shader_register 
{
  namespace file_ns = blender::nodes::node_shader_shape_cc;

  static blender::bke::bNodeType ntype;

  //all sh_node_... was ddeclared in this file
  sh_fn_node_type_base(&ntype, SH_NODE_SHAPE, "shape", NODE_CLASS_CONVERTER);
  ntype.declare = file_ns::sh_node_shape_declare;
  ntype.draw_buttons = file_ns::node_shader_buts_shape;
  ntype.initfunc = file_ns::node_shader_init_shape;
  ntype.gpu_fn = file_ns::gpu_shader_shape;
  ntype.build_multi_function = file_ns::sh_node_shape_build_multi_function;
  //ntype.materialx_fn = file_ns::node_shader_materialx;

  blender::bke::nodeRegisterType(&ntype);
}
