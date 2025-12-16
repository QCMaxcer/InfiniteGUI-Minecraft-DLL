#include "Motionblur.h"
#include <GL/glew.h>
#include <GL/GL.h>
#include "opengl_hook.h"
#include "GameStateDetector.h"
#include "imgui\imgui_internal.h"
#include "ImGuiStd.h"
auto vertex_shader_code = R"glsl(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)glsl";

auto fragment_shader_code = R"glsl(
#version 330 core

out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D currentTexture;
uniform sampler2D historyTexture;
uniform float blurriness;
uniform float velocity_factor;
uniform bool renderRGB;
uniform bool smooth_blur;

vec4 blurHistory(vec2 uv)
{
    float offset = 0.0005 * velocity_factor;
    vec4 sum = vec4(0.0);

    sum += texture(historyTexture, uv + vec2(-offset, 0.0)) * 0.25;
    sum += texture(historyTexture, uv + vec2( offset, 0.0)) * 0.25;
    sum += texture(historyTexture, uv + vec2(0.0, -offset)) * 0.25;
    sum += texture(historyTexture, uv + vec2(0.0,  offset)) * 0.25;

    return sum;
}

void main()
{
    vec4 current = texture(currentTexture, TexCoord);
    vec4 history = texture(historyTexture, TexCoord);
    float cur_blurriness = blurriness;
    vec4 blurredHistory = history;
	if(velocity_factor > 0.0){
		float base_blurriness = blurriness / 2.0;
		cur_blurriness = base_blurriness + (1.0 - velocity_factor) * base_blurriness;
		if(smooth_blur)
		{
			blurredHistory = blurHistory(TexCoord);
		}
	}
	vec4 blurredColor = mix(blurredHistory, current, cur_blurriness);
    
    if (renderRGB)
    {
        FragColor = blurredColor;
    }
    else
    {
		float value1 = current.r;
		FragColor = mix(blurredHistory, vec4(value1), cur_blurriness);
    }
}

)glsl";

void Motionblur::Toggle()
{
}

void Motionblur::Render()
{
	if (!isEnabled) return;
	static bool initialize = false;
	static bool first = true;

	const int width = opengl_hook::screen_size.x;
	const int height = opengl_hook::screen_size.y;

	GLint viewport[4];
	GLint prev_framebuffer = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prev_framebuffer);
	glGetIntegerv(GL_VIEWPORT, viewport);

	glViewport(0, 0, width, height);

	if (!initialize)
	{
		initialize_texture(width, height);
		initialize_quad();
		initialize_shader();
		initialize = true;

		texture_width_ = width;
		texture_height_ = height;
	}

	if (texture_width_ != width || texture_height_ != height)
	{
		resize_texture(width, height);
		first = true;
	}


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	copy_to_current();
	if (first)
	{
		copy_to_history();
		first = false;
	}

	//计算blurriness_value值
	if (velocityAdaptive)
		velocity_adaptive_blur(GameStateDetector::Instance().IsCameraMoving(),GameStateDetector::Instance().GetCameraSpeed(), &velocity_factor);
	else
		velocity_factor = 0.0f;

	if (FpsModulate)
		Fps_modulate(ImGui::GetIO().Framerate, &blurriness_value, &cur_blurriness_value);
	else
		cur_blurriness_value = blurriness_value;

	draw_texture();
	copy_to_history();

	glBindFramebuffer(GL_FRAMEBUFFER, prev_framebuffer);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void Motionblur::initialize_texture(const int width, const int height)
{
	glGenTextures(1, &current_texture_);
	glBindTexture(GL_TEXTURE_2D, current_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &history_texture_);
	glBindTexture(GL_TEXTURE_2D, history_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Motionblur::initialize_quad()
{
	constexpr GLfloat quad_vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &quad_vao_);
	glGenBuffers(1, &quad_vbo_);

	glBindVertexArray(quad_vao_);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), static_cast<GLvoid*>(0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
		reinterpret_cast<GLvoid*>(2 * sizeof(GLfloat)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Motionblur::initialize_shader()
{
	shader_program_ = glCreateProgram();

	const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_code, nullptr);
	glCompileShader(vertex_shader);

	const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_code, nullptr);
	glCompileShader(fragment_shader);

	glAttachShader(shader_program_, vertex_shader);
	glAttachShader(shader_program_, fragment_shader);
	glLinkProgram(shader_program_);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

void Motionblur::resize_texture(int width, int height)
{
	glBindTexture(GL_TEXTURE_2D, current_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, history_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, 0);

	texture_width_ = width;
	texture_height_ = height;
}

void Motionblur::draw_texture() const 
{
	if (current_texture_ == 0)
		return;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glUseProgram(shader_program_);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, current_texture_);
	glUniform1i(glGetUniformLocation(shader_program_, "currentTexture"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, history_texture_);
	glUniform1i(glGetUniformLocation(shader_program_, "historyTexture"), 1);

	auto value = max(0.1, (50 - cur_blurriness_value)) / 100;
	glUniform1f(glGetUniformLocation(shader_program_, "blurriness"), (GLfloat)value);
	glUniform1f(glGetUniformLocation(shader_program_, "velocity_factor"), (GLfloat)velocity_factor);
	glUniform1f(glGetUniformLocation(shader_program_, "renderRGB"), !clear_color);
	glUniform1f(glGetUniformLocation(shader_program_, "smooth_blur"), smooth_blur);

	glBindVertexArray(quad_vao_);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
	glUseProgram(0);
}

void Motionblur::copy_to_history() const
{
	glBindTexture(GL_TEXTURE_2D, history_texture_);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, texture_width_, texture_height_, 0);
}

void Motionblur::copy_to_current() const
{
	glBindTexture(GL_TEXTURE_2D, current_texture_);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, texture_width_, texture_height_, 0);
}

void Motionblur::velocity_adaptive_blur(bool cameraMoving, float cameraSpeed, float* velocity_factor)
{
	//cameraSpeed 1~10
	ImGuiIO& io = ImGui::GetIO();
	float lerpSpeed = 10.0f * io.DeltaTime;
	float tar_velocity_factor = std::clamp((cameraSpeed - 1.0f), 0.0f, 15.0f) / 15.0f;

	if (cameraMoving)
	{
		*velocity_factor = tar_velocity_factor;
	}
	else
	{
		*velocity_factor = ImLerp(*velocity_factor, 0.0f, lerpSpeed);
		if(*velocity_factor < 0.01f)
			*velocity_factor = 0.0f;
	}
}

void Motionblur::Fps_modulate(float fps, float* blurriness_value, float* cur_blurriness_value)
{
	*cur_blurriness_value = *blurriness_value * ImClamp(fps, 0.0f, 500.0f) / 500.0f; //fps越高，模糊强度越高
}

void Motionblur::Load(const nlohmann::json& j)
{
	LoadItem(j);
	if (j.contains("blurriness")) blurriness_value = j["blurriness"].get<float>();
	if (j.contains("velocityAdaptive")) velocityAdaptive = j["velocityAdaptive"].get<bool>();
	if (j.contains("smooth_blur")) smooth_blur = j["smooth_blur"].get<bool>();
	if (j.contains("applayOnMenu")) applayOnMenu = j["applayOnMenu"].get<bool>();
	if (j.contains("clear_color")) clear_color = j["clear_color"].get<bool>();
	if (j.contains("FpsModulate")) FpsModulate = j["FpsModulate"].get<bool>();
}

void Motionblur::Save(nlohmann::json& j) const
{
	SaveItem(j);
	j["blurriness"] = blurriness_value;
	j["velocityAdaptive"] = velocityAdaptive;
	j["smooth_blur"] = smooth_blur;
	j["applayOnMenu"] = applayOnMenu;
	j["clear_color"] = clear_color;
	j["FpsModulate"] = FpsModulate;
}

void Motionblur::DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth)
{
	//DrawItemSettings();
	ImGui::SliderFloat(u8"模糊强度", &blurriness_value, 0.0f, 40.0f, "%.1f");
	ImGui::Checkbox(u8"菜单动态模糊", &applayOnMenu);
	ImGui::Checkbox(u8"帧率调制", &FpsModulate);
	ImGui::SameLine(); ImGuiStd::HelpMarker(u8"模糊强度随着帧率变化，适用于帧率不稳定的情况。");
	if (ImGui::Checkbox(u8"速度自适应", &velocityAdaptive))
	{
		if (!velocityAdaptive) smooth_blur = false;
	}
	ImGui::SameLine(); ImGuiStd::HelpMarker(u8"根据视角移动速度调整模糊强度，能有效解决鬼影问题。");
	if (velocityAdaptive) { ImGui::Checkbox(u8"柔和模糊", &smooth_blur); ImGui::SameLine(); ImGuiStd::HelpMarker(u8"柔和化模糊拖影，但会使MCUI模糊。");  }
	ImGui::Checkbox(u8"Im Faded~", &clear_color);
}
