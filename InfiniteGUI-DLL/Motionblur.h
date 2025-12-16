#pragma once
#include "Item.h"
#include "RenderModule.h"
class Motionblur : public Item, public RenderModule {
public:
    Motionblur() {
        type = Visual; // 信息项类型
        name = u8"动态模糊";
        description = u8"采用帧混合技术实现的动态模糊";
        icon = u8"\uE059";
        Reset();
    }
    ~Motionblur() {}

    static Motionblur& Instance()
    {
        static Motionblur instance;
        return instance;
    }

    void Toggle() override;
    void Reset() override
    {
        isEnabled = false;
        blurriness_value = 10.0f;
        cur_blurriness_value = 10.0f;
        smooth_blur = true;
        clear_color = false;
        velocityAdaptive = true;
    }
    void Render() override;
    void Load(const nlohmann::json& j) override;
    void Save(nlohmann::json& j) const override;
    void DrawSettings(const float& bigPadding, const float& centerX, const float& itemWidth) override;

    void initialize_texture(const int width, const int height);
    void initialize_quad();
	void initialize_shader();

	void resize_texture(int width, int height);
	void draw_texture() const;
	void copy_to_history() const;
	void copy_to_current() const;

    static void velocity_adaptive_blur(bool cameraMoving, float velocity, float* velocity_factor);
    static void Fps_modulate(float fps, float* blurriness_value, float* cur_blurriness_value);
    bool applayOnMenu = true;
private:
    uint32_t shader_program_;

    uint32_t current_texture_;
    uint32_t history_texture_;

    uint32_t quad_vao_;
    uint32_t quad_vbo_;

    int32_t texture_width_;
    int32_t texture_height_;

    float velocity_factor = 0.0f;
    float cur_blurriness_value = 10.0f;
    float blurriness_value = 10.0f;
    bool smooth_blur = true;
    bool clear_color = false;
    bool velocityAdaptive = true;
    bool FpsModulate = true;

};