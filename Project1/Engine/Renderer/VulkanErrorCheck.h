#pragma once
#include <iostream>
#include <assert.h>

#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res == VK_SUCCESS){ }																			\
	else                                                                                                \
	{																									\
		std::cout << "Fatal : VkResult in " << __FILE__ << " at line " << __LINE__ << std::endl; \
		assert(res == VK_SUCCESS);																		\
	}																									\
}