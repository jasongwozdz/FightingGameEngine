#include "pch.h"
#define VMA_IMPLEMENTATION
#define GTEST_LANG_CXX11 1 //need to do this to get gtest to compile with visual studio 2017 https://stackoverflow.com/questions/42847103/stdtr1-with-visual-studio-2017
#include "gtest/gtest.h"
#include "../Project1/Engine/Scene/Componenets/Animator.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "../Project1/Engine/NewRenderer/VkRenderer.h"

class AnimatorTestFixture : public ::testing::Test
{
protected:

	glm::mat4 interpolatePos(AnimationClip clip, float time)
	{
		animator->localTime_ = time;
		return animator->interpolateTransforms(0, clip, 1);
	}

	void SetUp() override
	{
		AnimationClip clip1;
		clip1 = { "test1", 10, 1 };
		KeyPosition pos1 = { 0.0, {1.0f, 1.0f, 1.0f} };
		KeyPosition pos2 = { 1.0, {2.0f, 2.0f, 2.0f} };
		clip1.positions_.push_back({pos1, pos2});

		KeyScale scale1 = { 0.0, {1.0f} };
		KeyScale scale2 = { 1.0, {1.0f} };
		clip1.scale_.push_back({ scale1, scale2 });

		KeyRotation rot1 = { 0.0, {1.0f, 1.0f, 1.0f, 1.0f} };
		KeyRotation rot2 = { 1.0, {1.0f, 1.0f, 1.0f, 1.0f} };
		clip1.rotations_.push_back({ rot1, rot2 });

		clips.push_back(clip1);

		animator = new Animator(clips);
	}

	std::vector<AnimationClip> clips;
	Animator *animator;
};

TEST_F(AnimatorTestFixture, interpolatePositionTest)
{
	AnimationClip clip1;
	clip1 = { "test1", 10, 1 };
	KeyPosition pos1 = { 0.0, {1.0f, 1.0f, 1.0f} };
	KeyPosition pos2 = { 1.0, {2.0f, 2.0f, 2.0f} };
	clip1.positions_.push_back({pos1, pos2});

	KeyScale scale1 = { 0.0, {1.0f} };
	KeyScale scale2 = { 1.0, {1.0f} };
	clip1.scale_.push_back({ scale1, scale2 });

	KeyRotation rot1 = { 0.0, {1.0f, 1.0f, 1.0f, 1.0f} };
	KeyRotation rot2 = { 1.0, {1.0f, 1.0f, 1.0f, 1.0f} };
	clip1.rotations_.push_back({ rot1, rot2 });

	glm::mat4 output = interpolatePos(clip1, 0.0f);
	glm::vec3 scale;
	glm::quat orientation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(output, scale, orientation, translation, skew, perspective);

	glm::vec3 expected = { 1.0f, 1.0f, 1.0f };
	EXPECT_EQ(translation, expected);
	
	clip1;
	clip1 = { "test1", 10, 1 };
	pos1 = { 0.0, {1.0f, 1.0f, 1.0f} };
	pos2 = { 1.0, {2.0f, 2.0f, 2.0f} };
	clip1.positions_.push_back({pos1, pos2});

	scale1 = { 0.0, {1.0f} };
	scale2 = { 1.0, {1.0f} };
	clip1.scale_.push_back({ scale1, scale2 });

	rot1 = { 0.0, {1.0f, 1.0f, 1.0f, 1.0f} };
	rot2 = { 1.0, {1.0f, 1.0f, 1.0f, 1.0f} };
	clip1.rotations_.push_back({ rot1, rot2 });

	output = interpolatePos(clip1, 0.5f);
	scale;
	orientation;
	translation;
	skew;
	perspective;
	glm::decompose(output, scale, orientation, translation, skew, perspective);

	expected = { 1.5f, 1.5f, 1.5f };
	EXPECT_EQ(translation, expected);
}

TEST_F(AnimatorTestFixture, interpolateQuatTest)
{
	AnimationClip clip1;
	clip1 = { "test1", 10, 1 };
	KeyPosition startPos = { 0.0, {1.0f, 1.0f, 1.0f} };
	KeyPosition endPos = { 1.0, {1.0f, 1.0f, 1.0f} };
	clip1.positions_.push_back({startPos, endPos});

	KeyScale startScale = { 0.0, {1.0f} };
	KeyScale endScale = { 1.0, {1.0f} };
	clip1.scale_.push_back({ startScale, endScale });

	KeyRotation startRot = { 0.0, {0.0382808000f, 0.739188790f, -0.0618495345f, 0.669558942f} };
	KeyRotation endRot = { 1.0, {0.0449499451f, 0.725735605f, -0.0698880255f, 0.682937026f} };
	clip1.rotations_.push_back({ startRot, endRot });

	glm::mat4 output = interpolatePos(clip1, 1.17312312f);
	glm::vec3 scale;
	glm::quat orientation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(output, scale, orientation, translation, skew, perspective);

	glm::quat expected = { 0.0395735353, 0.736610889, -0.0634082779, 0.672173798 };
	EXPECT_FLOAT_EQ(orientation.x, expected.x);
	EXPECT_FLOAT_EQ(orientation.y, expected.y);
	EXPECT_FLOAT_EQ(orientation.z, expected.z);
	EXPECT_FLOAT_EQ(orientation.w, expected.w);
}
