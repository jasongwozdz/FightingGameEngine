#include "VkTypes.h"

bool operator==(const PipelineCreateInfo& left, const PipelineCreateInfo& right)
{
	PipelineCreateInfoHash hashFunc;
	return  hashFunc(left) == hashFunc(right);
}
