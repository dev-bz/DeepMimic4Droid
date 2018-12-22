#version 300 es
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#define USE_SHADOW 0
uniform sampler2D gTexture;
#if USE_SHADOW<2
uniform sampler2D gShadowTex;

uniform mat4 gShadowView;
uniform mat4 gShadowProj;
#endif
uniform vec3 gLightDir;
uniform vec3 gLightColour;
uniform vec3 gAmbientColour;
uniform vec4 gMaterialData;
uniform vec4 gFogColor;
uniform vec4 gFogData;

in vec3 ViewPos;
in vec3 Normal;
in vec4 VertColor;
in vec2 TexCoord;

out vec4 outColor;

float CalcDiffuse(
    vec3 world_space_normal,
    vec3 light_direction) // Roughness value [0 ~ 1], 0 is same as Lambertian
{
  vec3 n = world_space_normal;
  vec3 l = light_direction;
  float l_dot_n = dot(l, n);
  float diffuse = clamp(l_dot_n, 0.0, 1.0);
  return diffuse;
}

float CalcSpecular(const vec3 world_normal, const vec3 light_direction,
                   const vec3 view_direction) {

  vec3 n = world_normal;
  vec3 v = view_direction;
  vec3 l = light_direction;
  vec3 h = normalize(view_direction + light_direction);

  float n_dot_h = dot(n, h);
  float spec = clamp(n_dot_h, 0.0, 1.0);
  spec = pow(spec, 10.0);
  return 0.0;
}

vec3 CalculateBRDF(vec3 normal, vec3 light_dir, vec3 light_colour,
                   vec3 view_dir, float roughness, vec3 albedo) {
  float diffuse_coef = CalcDiffuse(normal, light_dir);
  float spec_coef = CalcSpecular(normal, light_dir, view_dir);

  return diffuse_coef * light_colour * albedo + spec_coef * light_colour;
}

vec2 RotateDirections(vec2 Dir, float theta) {
  float cos_theta = cos(theta);
  float sin_theta = sin(theta);
  return vec2(Dir.x * cos_theta - Dir.y * sin_theta,
              Dir.x * sin_theta + Dir.y * cos_theta);
}
#if USE_SHADOW<2
float CalculateShadow(vec3 view_pos, vec3 normal) {
  float bias = 0.02f;
  vec4 position = vec4(view_pos + bias * normal, 1.f);

  vec2 shadow_coord = (gShadowProj * position).xy;
  if (shadow_coord.x > 1.0 || shadow_coord.x < -1.0 || shadow_coord.y > 1.0 ||
      shadow_coord.y < -1.0)
    return 1.0;
  /*float depth = (shadow_coord.z * 0.5 + 0.5); // + 0.01f;
  depth = min(depth, 1.0);*/
  float depth =- (gShadowView * position).z;

  shadow_coord.xy = shadow_coord.xy * 0.5f + 0.5f;
#if USE_SHADOW==1
  float sample_depth = (depth < texture(gShadowTex, shadow_coord.xy).r) ? 1.f : 0.f;
#else
  float sample_depth = 0.0; // (texture(gShadowTex, shadow_coord.xy).r);
  //sample_depth = (depth < sample_depth) ? 1.f : 0.f;

  // pcf taps for anti-aliasing
  const int num_samples = 16;
  vec2 poissonDisk[num_samples] =
      vec2[](vec2(-0.6474742f, 0.6028621f), vec2(0.0939157f, 0.6783564f),
             vec2(-0.3371512f, 0.04865054f), vec2(-0.4010732f, 0.914994f),
             vec2(-0.2793565f, 0.4456959f), vec2(-0.6683437f, -0.1396244f),
             vec2(-0.6369296f, -0.6966243f), vec2(-0.2684143f, -0.3756073f),
             vec2(0.1146429f, -0.8692533f), vec2(0.0697926f, 0.01110036f),
             vec2(0.4677842f, 0.5375957f), vec2(0.377133f, -0.3518053f),
             vec2(0.6722369f, 0.03702459f), vec2(0.6890426f, -0.5889201f),
             vec2(-0.8208677f, 0.2444565f), vec2(0.8431721f, 0.3903837f));
  float shadow_map_size = 256.0;
  float r = 2.0;

  const vec4 noise_params = vec4(1, 1, 0.1, 0.2);
  vec2 seed = view_pos.xy * noise_params.xy + noise_params.zw;
  float rand_theta = fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43.5453);
  rand_theta = 2.0 * 3.14159f * rand_theta - 3.14159f;

  for (int i = 0; i < num_samples; ++i) {
    vec2 dir = poissonDisk[i];
    dir = RotateDirections(dir, rand_theta);
    vec2 tap_coord = shadow_coord.xy + r * dir / shadow_map_size;
    float tap = texture(gShadowTex, tap_coord).r;
    sample_depth += (depth <= tap) ? 0.0625: 0.0;
  }
#endif
  //float shadow_coef = sample_depth / float(num_samples);
  return sample_depth;
}
#endif
vec3 CalcAmbient(vec3 normal, vec3 albedo) {
  vec3 ambient = gAmbientColour;
  // ambient = vec3(0.6, 0.6, 0.6);
  ambient *= albedo;
  return ambient;
}

float CalcFog(vec3 pos) {
  float fog_cutoff = gFogData.x;
  float fade_distsq = gFogData.y;
  float dist = length(pos);
  dist = max(0.0, dist - fog_cutoff);
  float fog = 1.0 - exp(-fade_distsq * dist * dist);
  return fog;
}

void main() {
#ifndef CAFFE
  float roughness = gMaterialData.x;
  bool enable_albedo_tex = gMaterialData.y != 0.0;
  vec3 view_dir = -normalize(ViewPos);
  vec3 norm = normalize(Normal);

  vec3 albedo = VertColor.rgb;
  if (enable_albedo_tex) {
    vec4 tex_col = texture(gTexture, TexCoord);
    albedo.rgb = albedo.rgb * tex_col.rgb;
  }
#if USE_SHADOW<2
  float shadow_coef = CalculateShadow(ViewPos, norm);
#else
  float shadow_coef = 1.0;
#endif
  vec3 light_colour = gLightColour;
  // light_colour = vec3(0.5, 0.5, 0.5);
  light_colour *= shadow_coef;
  vec3 light_result =
      CalculateBRDF(norm, gLightDir, light_colour, view_dir, roughness, albedo);

  vec3 ambient = CalcAmbient(norm, albedo.rgb);
  light_result += ambient;

  float fog = CalcFog(ViewPos);
  fog *= gFogColor.w;
  light_result = mix(light_result, gFogColor.xyz, fog);

  outColor = vec4(light_result, VertColor[3]);
#else

  vec3 n = normalize(Normal); // Compute curvature
  vec3 dx = dFdx(n);
  vec3 dy = dFdy(n);
  vec3 xneg = n - dx;
  vec3 xpos = n + dx;
  vec3 yneg = n - dy;
  vec3 ypos = n + dy;
  float depth = length(ViewPos);
  float curvature = (cross(xneg, xpos).y - cross(yneg, ypos).x) * 4.0 /
                    depth; // Compute surface properties
  float corrosion = clamp(-curvature * 3.0, 0.0, 1.0);
  float shine = clamp(curvature * 5.0, 0.0, 1.0);
  vec3 light = normalize(gLightDir);
  vec3 ambient = vec3(0.15, 0.1, 0.1);
  vec3 diffuse = mix(mix(vec3(0.3, 0.25, 0.2), vec3(0.45, 0.5, 0.5), corrosion),
                     vec3(0.5, 0.4, 0.3), shine) -
                 ambient;
  vec3 specular = mix(vec3(0.0), vec3(1.0) - ambient - diffuse, shine);
  float shininess = 32.0; // Compute final color
  float cosAngle = dot(n, light);
#if USE_SHADOW<2
  float shadow_coef = CalculateShadow(ViewPos, n);
#else
  float shadow_coef = 1.0;
#endif
  outColor.rgb= ambient + shadow_coef * diffuse * max(0.0, cosAngle) +
          specular * pow(max(0.0, cosAngle), shininess);
outColor.a=1.0;
  #endif
}