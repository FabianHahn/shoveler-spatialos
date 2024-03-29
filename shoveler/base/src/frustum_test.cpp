#include <gtest/gtest.h>

#include <cmath>
#include <map>
#include <string>
#include <utility>

extern "C" {
#include "shoveler/constants.h"
#include "shoveler/frustum.h"
#include "shoveler/projection.h"
#include "shoveler/types.h"
}

static const float eps = 1e-6f;

static bool operator==(const ShovelerVector3& a, const ShovelerVector3& b) {
  ShovelerVector3 diff = shovelerVector3LinearCombination(1.0f, a, -1.0f, b);
  float length2 = shovelerVector3Dot(diff, diff);

  return sqrtf(length2) < eps;
}

static std::ostream& operator<<(std::ostream& stream, const ShovelerVector3& v) {
  return stream << "(" << v.values[0] << ", " << v.values[1] << ", " << v.values[2] << ")";
}

class ShovelerFrustumTest : public ::testing::Test {
public:
  virtual void SetUp() {
    projection.fieldOfViewY = 2.0f * SHOVELER_PI * 50.0f / 360.0f;
    projection.aspectRatio = 640.0f / 480.0f;
    projection.nearClippingPlane = 1.0f;
    projection.farClippingPlane = 10.0f;

    frame.position = shovelerVector3(0, 0, -5);
    frame.direction = shovelerVector3(0, 0, 1);
    frame.up = shovelerVector3(0, 1, 0);

    shovelerProjectionPerspectiveComputeFrustum(&projection, &frame, &frustum);
  }

  virtual void TearDown() {}

  ShovelerProjectionPerspective projection;
  ShovelerReferenceFrame frame;
  ShovelerFrustum frustum;
};

TEST_F(ShovelerFrustumTest, computeFrustum) {
  ShovelerMatrix projectionTransformation;
  shovelerProjectionPerspectiveComputeTransformation(&projection, &projectionTransformation);

  ShovelerMatrix viewTransformation;
  shovelerMatrixCreateLookIntoDirectionTransformation(&frame, &viewTransformation);

  ShovelerMatrix projectionView =
      shovelerMatrixMultiply(projectionTransformation, viewTransformation);

  ASSERT_EQ(
      shovelerMatrixMultiplyVector3(projectionView, frustum.nearBottomLeftVertex),
      shovelerVector3(-1.0f, -1.0f, -1.0f));
  ASSERT_EQ(
      shovelerMatrixMultiplyVector3(projectionView, frustum.nearBottomRightVertex),
      shovelerVector3(1.0f, -1.0f, -1.0f));
  ASSERT_EQ(
      shovelerMatrixMultiplyVector3(projectionView, frustum.nearTopRightVertex),
      shovelerVector3(1.0f, 1.0f, -1.0f));
  ASSERT_EQ(
      shovelerMatrixMultiplyVector3(projectionView, frustum.nearTopLeftVertex),
      shovelerVector3(-1.0f, 1.0f, -1.0f));
  ASSERT_EQ(
      shovelerMatrixMultiplyVector3(projectionView, frustum.farBottomLeftVertex),
      shovelerVector3(-1.0f, -1.0f, 1.0f));
  ASSERT_EQ(
      shovelerMatrixMultiplyVector3(projectionView, frustum.farBottomRightVertex),
      shovelerVector3(1.0f, -1.0f, 1.0f));
  ASSERT_EQ(
      shovelerMatrixMultiplyVector3(projectionView, frustum.farTopRightVertex),
      shovelerVector3(1.0f, 1.0f, 1.0f));
  ASSERT_EQ(
      shovelerMatrixMultiplyVector3(projectionView, frustum.farTopLeftVertex),
      shovelerVector3(-1.0f, 1.0f, 1.0f));

  ShovelerVector3 origin = shovelerVector3(0.0f, 0.0f, 0.0f);
  ASSERT_TRUE(shovelerPlaneVectorDistance(frustum.nearPlane, origin) < 0.0f);
  ASSERT_TRUE(shovelerPlaneVectorDistance(frustum.farPlane, origin) < 0.0f);
  ASSERT_TRUE(shovelerPlaneVectorDistance(frustum.leftPlane, origin) < 0.0f);
  ASSERT_TRUE(shovelerPlaneVectorDistance(frustum.bottomPlane, origin) < 0.0f);
  ASSERT_TRUE(shovelerPlaneVectorDistance(frustum.rightPlane, origin) < 0.0f);
  ASSERT_TRUE(shovelerPlaneVectorDistance(frustum.topPlane, origin) < 0.0f);

  ASSERT_TRUE(
      shovelerPlaneVectorDistance(frustum.nearPlane, shovelerVector3(0.0f, 0.0f, -10.0f)) > 0.0f);
  ASSERT_TRUE(
      shovelerPlaneVectorDistance(frustum.farPlane, shovelerVector3(0.0f, 0.0f, 10.0f)) > 0.0f);
  ASSERT_TRUE(
      shovelerPlaneVectorDistance(frustum.leftPlane, shovelerVector3(5.0f, 0.0f, 0.0f)) > 0.0f);
  ASSERT_TRUE(
      shovelerPlaneVectorDistance(frustum.bottomPlane, shovelerVector3(0.0f, -5.0f, 0.0f)) > 0.0f);
  ASSERT_TRUE(
      shovelerPlaneVectorDistance(frustum.rightPlane, shovelerVector3(-5.0f, 0.0f, 0.0f)) > 0.0f);
  ASSERT_TRUE(
      shovelerPlaneVectorDistance(frustum.topPlane, shovelerVector3(0.0f, 5.0f, 0.0f)) > 0.0f);
}

TEST_F(ShovelerFrustumTest, intersectFrustumWithFrustumSelf) {
  ASSERT_TRUE(shovelerFrustumIntersectFrustum(&frustum, &frustum));
}

TEST_F(ShovelerFrustumTest, intersectFrustumWithFrustumPositive) {
  ShovelerReferenceFrame otherFrame;
  otherFrame.position = shovelerVector3(0, 0, 5);
  otherFrame.direction = shovelerVector3(0, 0, -1);
  otherFrame.up = shovelerVector3(0, -1, 0);

  ShovelerFrustum otherFrustum;
  shovelerProjectionPerspectiveComputeFrustum(&projection, &otherFrame, &otherFrustum);

  ASSERT_TRUE(shovelerFrustumIntersectFrustum(&frustum, &otherFrustum));
  ASSERT_TRUE(shovelerFrustumIntersectFrustum(&otherFrustum, &frustum));
}

TEST_F(ShovelerFrustumTest, intersectFrustumWithFrustumNegative) {
  ShovelerReferenceFrame otherFrame;
  otherFrame.position = shovelerVector3(0, 0, 11);
  otherFrame.direction = shovelerVector3(0, 0, 1);
  otherFrame.up = shovelerVector3(0, -1, 0);

  ShovelerFrustum otherFrustum;
  shovelerProjectionPerspectiveComputeFrustum(&projection, &otherFrame, &otherFrustum);

  ASSERT_FALSE(shovelerFrustumIntersectFrustum(&frustum, &otherFrustum));
  ASSERT_FALSE(shovelerFrustumIntersectFrustum(&otherFrustum, &frustum));
}
