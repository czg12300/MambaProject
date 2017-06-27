#ifndef _E_FRAMECTRL_H_
#define _E_FRAMECTRL_H_

namespace e
{
	class FrameCtrl
	{
	public:
		FrameCtrl(void);
		virtual ~FrameCtrl(void);
	public:
		enum State{
			STATE_IDLE,		//
			STATE_RENDER,   //
			STATE_SKIP		//
		};

		void Reset(void);
		State Step(void);
		void SetFramePerSecond(int nFrameCount)
		{
			m_nFramePerSecond = nFrameCount;
		}
		double GetRenderFPS(void) const
		{
			return m_lfRenderFPS;
		}
		double GetSkipedFPS(void) const
		{
			return m_lfSkipedFPS;
		}
		double GetLoad(void) const
		{
			return m_lfLoad;
		}
		bool IsThisCycleLastFrame(void) const
		{
			return m_bThisCycleLastFrame;
		}

	private:
		int    m_nFramePerSecond;
		int    m_nContinuousSkipCount;
		double m_lfRenderFrameCount;
		double m_lfSkipedFrameCount;
		double m_lfBusyTimeSpan;
		double m_lfRenderFPS;
		double m_lfSkipedFPS;
		double m_lfThisCycleT0;
		double m_lfDeltaTime;
		State  m_LastState;
		double m_lfLoad;	
		double m_lfPrevT0;
		bool   m_bThisCycleLastFrame;
	};

}

#endif