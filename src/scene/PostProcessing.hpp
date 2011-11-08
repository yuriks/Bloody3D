#pragma once

#include "scene/Scene.hpp"
#include "scene/RenderContext.hpp"
#include "mesh/Material.hpp"

namespace scene {

void tonemap(const ShadingBufferSet& shading_buffers, const Material& material, const RenderContext& render_context);

} // namespace scene
