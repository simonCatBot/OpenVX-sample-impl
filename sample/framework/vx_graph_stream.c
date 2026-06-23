/*
 * Copyright (c) 2012-2019 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef OPENVX_USE_STREAMING

#include <VX/vx.h>
#include <VX/vx_khr_pipelining.h>
#include <VX/vx_compatibility.h>

#include "vx_internal.h"

static vx_value_t ownStreamingThread(void *arg)
{
    vx_graph graph = (vx_graph)arg;
    vx_status status = VX_SUCCESS;

    while (graph->streaming_stop == vx_false_e)
    {
        status = vxProcessGraph(graph);
        if (status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Streaming graph execution failed with status %d\n", status);
            break;
        }
    }

    return (vx_value_t)0;
}

VX_API_ENTRY vx_status VX_API_CALL vxEnableGraphStreaming(vx_graph graph,
                vx_node trigger_node)
{
    if (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;

    if (graph->verified == vx_true_e)
        return VX_ERROR_NOT_SUPPORTED;

    if (trigger_node != NULL &&
        ownIsValidSpecificReference(&trigger_node->base, VX_TYPE_NODE) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;

    graph->streaming_enabled = vx_true_e;
    graph->streaming_trigger_node = trigger_node;

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxStartGraphStreaming(vx_graph graph)
{
    if (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;

    if (graph->streaming_enabled == vx_false_e)
        return VX_FAILURE;

    if (graph->verified == vx_false_e)
        return VX_FAILURE;

    if (graph->streaming_thread_running == vx_true_e)
        return VX_FAILURE;

    graph->streaming_stop = vx_false_e;
    graph->streaming_thread = ownCreateThread(ownStreamingThread, graph);
    if (graph->streaming_thread == 0)
        return VX_FAILURE;

    graph->streaming_thread_running = vx_true_e;
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxStopGraphStreaming(vx_graph graph)
{
    if (ownIsValidSpecificReference(&graph->base, VX_TYPE_GRAPH) == vx_false_e)
        return VX_ERROR_INVALID_REFERENCE;

    if (graph->streaming_thread_running == vx_false_e)
        return VX_FAILURE;

    graph->streaming_stop = vx_true_e;

    if (graph->streaming_thread)
    {
        ownJoinThread(graph->streaming_thread, NULL);
        graph->streaming_thread = 0;
    }

    graph->streaming_thread_running = vx_false_e;
    graph->streaming_enabled = vx_false_e;
    graph->streaming_trigger_node = NULL;

    return VX_SUCCESS;
}

#endif
