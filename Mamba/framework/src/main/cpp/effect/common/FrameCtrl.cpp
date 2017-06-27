#include "FrameCtrl.h"


namespace e 
{
	inline double GetTicks(void)
	{
//		timeval t;
//		gettimeofday(&t, NULL);
//		return double(t.tv_sec) + double(t.tv_usec) / 1000000.0;
        return 0;
	}

	FrameCtrl::FrameCtrl()
	{
		Reset();
	}

	FrameCtrl::~FrameCtrl()
	{

	}

	void FrameCtrl::Reset(void)
	{
		m_nContinuousSkipCount = 0;
		m_lfRenderFrameCount = 0;
		m_lfSkipedFrameCount = 0;
		m_lfRenderFPS = 0;
		m_lfSkipedFPS = 0;
		m_lfThisCycleT0 = GetTicks();
		m_LastState = STATE_IDLE;
		m_lfLoad = 1.0;
		m_lfBusyTimeSpan = 0;
		m_lfPrevT0 = GetTicks();
		//15֡ÿ��
		m_nFramePerSecond = 25;
		m_bThisCycleLastFrame = false;
		m_lfDeltaTime = 1000 / m_nFramePerSecond;
	}

	FrameCtrl::State FrameCtrl::Step(void)
	{
		double t = GetTicks();

		if (m_LastState == STATE_SKIP || m_LastState == STATE_RENDER)
		{
			m_lfBusyTimeSpan += t - m_lfPrevT0;
		}

		// ÿ�����һ��FPS�͸���
		double lfRenderFrameSpan = t - m_lfThisCycleT0;

		if (m_lfRenderFrameCount >= m_nFramePerSecond || lfRenderFrameSpan >= 1.0)
		{
			m_lfThisCycleT0 = t;
			m_lfRenderFPS = m_lfRenderFrameCount / lfRenderFrameSpan;
			m_lfSkipedFPS = m_lfSkipedFrameCount / lfRenderFrameSpan;

			if (m_lfRenderFrameCount > 0)
			{
				double lfNewLoad = m_lfBusyTimeSpan * m_nFramePerSecond / lfRenderFrameSpan / m_lfRenderFrameCount;
				m_lfLoad = lfNewLoad;
			}

			lfRenderFrameSpan = 0;
			m_lfRenderFrameCount = 0;
			m_lfSkipedFrameCount = 0;
			m_lfBusyTimeSpan = 0;
			m_bThisCycleLastFrame = true;
		}
		else if (lfRenderFrameSpan < 0)
		{
			m_lfThisCycleT0 = t;
			lfRenderFrameSpan = 0;
			m_bThisCycleLastFrame = false;
		}
		else
		{
			m_bThisCycleLastFrame = false;
		}

		m_lfPrevT0 = t;
		double lfFrameShouldBe = lfRenderFrameSpan * m_nFramePerSecond;
		// ���̫�����Ϣһ��, �����0.375�Ǿ������ֵ
		// TODO: ��ֱͬ��ʱ, Load�����ܲ���
		if (lfFrameShouldBe < m_lfRenderFrameCount + (1.1 - m_lfLoad) * 0.375)
		{
			m_lfSkipedFrameCount++;
			m_LastState = STATE_IDLE;
			return STATE_IDLE;
		}

		m_lfRenderFrameCount++;
		m_LastState = STATE_RENDER;
		return STATE_RENDER;
	}
}