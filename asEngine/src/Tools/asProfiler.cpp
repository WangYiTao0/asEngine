#include "aspch.h"
#include "asProfiler.h"
#include "Graphics\API\asGraphicsDevice.h"
#include "Graphics\asRenderer.h"
#include "Graphics\asFont.h"
#include "Helpers\asTimer.h"
#include "Helpers\asResourceManager.h"

using namespace asGraphics;

namespace asProfiler
{
	bool ENABLED = false;
	bool initialized = false;
	std::mutex lock;
	range_id cpu_frame;
	range_id gpu_frame;

	struct Range
	{
		std::string name;
		float time = 0;
		CommandList cmd = COMMANDLIST_COUNT;

		asTimer cpuBegin, cpuEnd;

		asRenderer::GPUQueryRing<4> gpuBegin;
		asRenderer::GPUQueryRing<4> gpuEnd;

		bool IsCPURange() const { return cmd == COMMANDLIST_COUNT; }
	};
	std::unordered_map<size_t, Range*> ranges;
	asRenderer::GPUQueryRing<4> disjoint;

	void BeginFrame()
	{
		if (!ENABLED)
			return;

		if (!initialized)
		{
			initialized = true;

			ranges.reserve(100);

			GPUQueryDesc desc;
			desc.Type = GPU_QUERY_TYPE_TIMESTAMP_DISJOINT;
			disjoint.Create(asRenderer::GetDevice(), &desc);
		}

		CommandList cmd = asRenderer::GetDevice()->BeginCommandList(); // it would be a good idea to not start a new command list just for these couple of queries!
		asRenderer::GetDevice()->QueryBegin(disjoint.Get_GPU(), cmd);
		asRenderer::GetDevice()->QueryEnd(disjoint.Get_GPU(), cmd); // this should be at the end of frame, but the problem is that there will be other command lists submitted in between and it doesn't work that way in DX11

		cpu_frame = BeginRangeCPU("CPU Frame");
		gpu_frame = BeginRangeGPU("GPU Frame", cmd);
	}
	void EndFrame(CommandList cmd)
	{
		if (!ENABLED || !initialized)
			return;

		// note: read the GPU Frame end range manually because it will be on a separate command list than start point:
		asRenderer::GetDevice()->QueryEnd(ranges[gpu_frame]->gpuEnd.Get_GPU(), cmd);

		EndRange(cpu_frame);

		GPUQueryResult disjoint_result;
		GPUQuery* disjoint_query = disjoint.Get_CPU();
		if (disjoint_query != nullptr)
		{
			while (!asRenderer::GetDevice()->QueryRead(disjoint_query, &disjoint_result));
		}

		for (auto& x : ranges)
		{
			auto& range = x.second;

			range->time = 0;
			if (range->IsCPURange())
			{
				range->time = (float)abs(range->cpuEnd.elapsed() - range->cpuBegin.elapsed());
			}
			else
			{
				GPUQuery* begin_query = range->gpuBegin.Get_CPU();
				GPUQuery* end_query = range->gpuEnd.Get_CPU();
				GPUQueryResult begin_result, end_result;
				if (begin_query != nullptr && end_query != nullptr)
				{
					while (!asRenderer::GetDevice()->QueryRead(begin_query, &begin_result));
					while (!asRenderer::GetDevice()->QueryRead(end_query, &end_result));
				}
				range->time = abs((float)(end_result.result_timestamp - begin_result.result_timestamp) / disjoint_result.result_timestamp_frequency * 1000.0f);
			}
		}
	}

	range_id BeginRangeCPU(const asHashString& name)
	{
		if (!ENABLED || !initialized)
			return 0;

		range_id id = name.GetHash();

		lock.lock();
		if (ranges.find(id) == ranges.end())
		{
			Range* range = new Range;
			range->name = name.GetString();
			range->time = 0;

			range->cpuBegin.Start();
			range->cpuEnd.Start();

			ranges.insert(std::make_pair(id, range));
		}

		ranges[id]->cpuBegin.record();

		lock.unlock();

		return id;
	}
	range_id BeginRangeGPU(const asHashString& name, CommandList cmd)
	{
		if (!ENABLED || !initialized)
			return 0;

		range_id id = name.GetHash();

		lock.lock();
		if (ranges.find(id) == ranges.end())
		{
			Range* range = new Range;
			range->name = name.GetString();
			range->time = 0;

			GPUQueryDesc desc;
			desc.Type = GPU_QUERY_TYPE_TIMESTAMP;
			range->gpuBegin.Create(asRenderer::GetDevice(), &desc);
			range->gpuEnd.Create(asRenderer::GetDevice(), &desc);

			ranges.insert(std::make_pair(id, range));
		}

		ranges[id]->cmd = cmd;
		asRenderer::GetDevice()->QueryEnd(ranges[id]->gpuBegin.Get_GPU(), cmd);

		lock.unlock();

		return id;
	}
	void EndRange(range_id id)
	{
		if (!ENABLED || !initialized)
			return;

		lock.lock();

		auto& it = ranges.find(id);
		if (it != ranges.end())
		{
			if (it->second->IsCPURange())
			{
				it->second->cpuEnd.record();
			}
			else
			{
				asRenderer::GetDevice()->QueryEnd(it->second->gpuEnd.Get_GPU(), it->second->cmd);
			}
		}
		else
		{
			assert(0);
		}

		lock.unlock();
	}

	void DrawData(int x, int y, CommandList cmd)
	{
		if (!ENABLED || !initialized)
			return;

		std::stringstream ss("");
		ss.precision(2);
		ss << "Frame Profiler Ranges:" << std::endl << "----------------------------" << std::endl;

		// Print CPU ranges:
		for (auto& x : ranges)
		{
			if (x.second->IsCPURange())
			{
				ss << x.second->name << ": " << std::fixed << x.second->time << " ms" << std::endl;
			}
		}
		ss << std::endl;

		// Print GPU ranges:
		for (auto& x : ranges)
		{
			if (!x.second->IsCPURange())
			{
				ss << x.second->name << ": " << std::fixed << x.second->time << " ms" << std::endl;
			}
		}

		asFont(ss.str(), asFontParams(x, y, ASFONTSIZE_DEFAULT, WIFALIGN_LEFT, WIFALIGN_TOP, 0, 0, asColor(255, 255, 255, 255), asColor(0, 0, 0, 255))).Draw(cmd);
	}

	void SetEnabled(bool value)
	{
		ENABLED = value;
	}
}





