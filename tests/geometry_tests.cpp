#include <gtest/gtest.h>
#include "../Constructs3D.h"
#include <glm/glm.hpp>

// Test triangle area calculation
TEST(GeometryTest, TriangleArea) {
    triangle tri;
    tri.p[0] = {0.0f, 0.0f, 0.0f, 1.0f};
    tri.p[1] = {1.0f, 0.0f, 0.0f, 1.0f};
    tri.p[2] = {0.0f, 1.0f, 0.0f, 1.0f};
    
    // Area of right triangle with base 1 and height 1 should be 0.5
    EXPECT_NEAR(tri.area(), 0.5f, 1e-5f);
}

// Test point inside triangle
TEST(GeometryTest, TriangleContains) {
    triangle tri;
    tri.p[0] = {0.0f, 0.0f, 0.0f, 1.0f};
    tri.p[1] = {1.0f, 0.0f, 0.0f, 1.0f};
    tri.p[2] = {0.0f, 1.0f, 0.0f, 1.0f};
    
    EXPECT_TRUE(tri.contains({0.2f, 0.2f, 0.0f, 1.0f}));
    EXPECT_FALSE(tri.contains({0.6f, 0.6f, 0.0f, 1.0f}));
    EXPECT_FALSE(tri.contains({-0.1f, 0.1f, 0.0f, 1.0f}));
}

// Test Shape generation (Rectangle)
TEST(GeometryTest, RectangleGeneration) {
    rectangle rect(2.0f, 3.0f); // width=2, height=3
    
    EXPECT_EQ(rect.type, shapetype::RECTANGLE);
    EXPECT_EQ(rect.triangles.size(), 2);
    
    // Check dimensions from triangles
    // tri1: (0,h), (w,h), (w,0)
    // tri2: (0,h), (w,0), (0,0)
    float minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for (const auto& tri : rect.triangles) {
        for (int i = 0; i < 3; i++) {
            minX = std::min(minX, tri.p[i].x);
            maxX = std::max(maxX, tri.p[i].x);
            minY = std::min(minY, tri.p[i].y);
            maxY = std::max(maxY, tri.p[i].y);
        }
    }
    
    EXPECT_FLOAT_EQ(minX, 0.0f);
    EXPECT_FLOAT_EQ(maxX, 2.0f);
    EXPECT_FLOAT_EQ(minY, 0.0f);
    EXPECT_FLOAT_EQ(maxY, 3.0f);
}

// Test Cube generation
TEST(GeometryTest, CubeGeneration) {
    cube c(1.0f); // size=1
    
    EXPECT_EQ(c.type, shapetype::CUBE);
    EXPECT_EQ(c.triangles.size(), 12); // 6 faces * 2 triangles per face
    
    // Cube is centered at origin by default in constructor (using s = size/2)
    float minX = 1e9, maxX = -1e9;
    for (const auto& tri : c.triangles) {
        for (int i = 0; i < 3; i++) {
            minX = std::min(minX, tri.p[i].x);
            maxX = std::max(maxX, tri.p[i].x);
        }
    }
    
    EXPECT_FLOAT_EQ(minX, -0.5f);
    EXPECT_FLOAT_EQ(maxX, 0.5f);
}

// Test Tangent calculation
TEST(GeometryTest, TangentCalculation) {
    triangle tri;
    // CCW Rectangle face on XY plane
    tri.p[0] = {0.0f, 1.0f, 0.0f, 1.0f}; tri.t[0] = {0.0f, 0.0f, 1.0f};
    tri.p[1] = {1.0f, 1.0f, 0.0f, 1.0f}; tri.t[1] = {1.0f, 0.0f, 1.0f};
    tri.p[2] = {1.0f, 0.0f, 0.0f, 1.0f}; tri.t[2] = {1.0f, 1.0f, 1.0f};
    
    glm::vec3 tangent = tri.calcTangent();
    
    // For this configuration, tangent should be along the X axis
    EXPECT_NEAR(tangent.x, 1.0f, 1e-5f);
    EXPECT_NEAR(tangent.y, 0.0f, 1e-5f);
    EXPECT_NEAR(tangent.z, 0.0f, 1e-5f);
}
