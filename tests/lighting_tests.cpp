#include <gtest/gtest.h>
#include "../Light.h"
#include <glm/glm.hpp>

// Test basic Light properties
TEST(LightingTest, DirectionalLightProperties) {
    Light light;
    light.color = glm::vec3(1.0f, 0.5f, 0.2f);
    light.diffuseIntensity = 0.8f;
    
    EXPECT_FLOAT_EQ(light.color.r, 1.0f);
    EXPECT_FLOAT_EQ(light.color.g, 0.5f);
    EXPECT_FLOAT_EQ(light.color.b, 0.2f);
    EXPECT_FLOAT_EQ(light.diffuseIntensity, 0.8f);
    EXPECT_EQ(light.name, "directional");
}

// Test PointLight attenuation and cutoff distance
TEST(LightingTest, PointLightAttenuation) {
    // kc=1.0, kl=0.7, kq=1.8
    PointLight pl(glm::vec3(0, 0, 0), 1.0f, 0.7f, 1.8f);
    
    EXPECT_EQ(pl.name, "point");
    EXPECT_GT(pl.cutoffDistance, 0.0f);
    
    // Test that a high attenuation results in a smaller cutoff distance
    PointLight plHigh(glm::vec3(0, 0, 0), 1.0f, 2.0f, 5.0f);
    EXPECT_LT(plHigh.cutoffDistance, pl.cutoffDistance);
}

// Test SpotLight properties
TEST(LightingTest, SpotLightProperties) {
    SpotLight sl(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), 1.0f, 0.09f, 0.032f, 12.5f, 15.0f);
    
    EXPECT_EQ(sl.name, "spot");
    EXPECT_FLOAT_EQ(sl.cutoff, 12.5f);
    EXPECT_FLOAT_EQ(sl.outerCutoff, 15.0f);
    EXPECT_FLOAT_EQ(sl.direction.y, -1.0f);
}
