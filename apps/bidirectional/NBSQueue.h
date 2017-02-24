//
//  NBSQueue.h
//  hog2 glut
//
//  Created by Nathan Sturtevant on 2/10/17.
//  Copyright © 2017 University of Denver. All rights reserved.
//

#ifndef NBSQueue_h
#define NBSQueue_h

#include "BDOpenClosed.h"

//low g -> low f
template <class state>
struct NBSCompareOpenReady {
	bool operator()(const BDOpenClosedData<state> &i1, const BDOpenClosedData<state> &i2) const
	{
		double f1 = i1.g + i1.h;
		double f2 = i2.g + i2.h;
		
		if (fequal(i1.g, i2.g))
		{
			return (!fless(f1, f2));
		}
		return (fgreater(i1.g, i2.g)); // low g over high
	}
};

template <class state>
struct NBSCompareOpenWaiting {
	bool operator()(const BDOpenClosedData<state> &i1, const BDOpenClosedData<state> &i2) const
	{
		double f1 = i1.g + i1.h;
		double f2 = i2.g + i2.h;
		
		if (fequal(f1, f2))
		{
			return (!fgreater(i1.g, i2.g));
		}
		return (fgreater(f1, f2)); // low f over high
	}
};

template <typename state, int epsilon = 1>
class NBSQueue {
public:
	bool GetNextPair(uint64_t &nextForward, uint64_t &nextBackward)
	{
		if (forwardQueue.OpenSize() == 0)
			return false;
		if (backwardQueue.OpenSize() == 0)
			return false;

		//instead of moving f<CLowerBound to ready immediately, moving lazily
		if (forwardQueue.OpenReadySize() == 0)
			forwardQueue.PutToReady();
		if (backwardQueue.OpenReadySize() == 0)
			backwardQueue.PutToReady();

		double fBound = DBL_MAX;
		while (true)
		{
			fBound = DBL_MAX;
			if (forwardQueue.OpenWaitingSize() != 0)
			{
				const auto i3 = forwardQueue.PeekAt(kOpenWaiting);
				fBound = std::min(fBound, i3.g + i3.h);
			}
			if (backwardQueue.OpenWaitingSize() != 0)
			{
				const auto i4 = backwardQueue.PeekAt(kOpenWaiting);
				fBound = std::min(fBound, i4.g + i4.h);
			}

			if (!fgreater(forwardQueue.PeekAt(kOpenReady).g + backwardQueue.PeekAt(kOpenReady).g + epsilon, fBound))
			{
				ComputeLowerBound();
				nextForward = forwardQueue.Peek(kOpenReady);
				nextBackward = backwardQueue.Peek(kOpenReady);
				return true;
			}

			if (forwardQueue.OpenWaitingSize() == 0 && backwardQueue.OpenWaitingSize() == 0)
			{
				// do nothing
			}
			else if (backwardQueue.OpenWaitingSize() == 0)
			{
				forwardQueue.PutToReady();
			}
			else if (forwardQueue.OpenWaitingSize() == 0)
			{
				backwardQueue.PutToReady();
			}
			else
			{
				const auto i1 = forwardQueue.PeekAt(kOpenWaiting);
				const auto i2 = backwardQueue.PeekAt(kOpenWaiting);
				if (fless(i1.g + i1.h, i2.g+ i2.h))
					forwardQueue.PutToReady();
				else
					backwardQueue.PutToReady();
			}
		}
		return false;
	}
	void Reset()
	{
		CLowerBound = 0;
		forwardQueue.Reset();
		backwardQueue.Reset();
	}
	double GetLowerBound() { return CLowerBound; }
	void ComputeLowerBound() 
	{
		CLowerBound = std::max(forwardQueue.PeekAt(kOpenReady).g + forwardQueue.PeekAt(kOpenReady).h,
			backwardQueue.PeekAt(kOpenReady).g+ backwardQueue.PeekAt(kOpenReady).h);
		CLowerBound = std::max(CLowerBound, forwardQueue.PeekAt(kOpenReady).g + backwardQueue.PeekAt(kOpenReady).g + epsilon);
	}
	BDOpenClosed<state, NBSCompareOpenReady<state>, NBSCompareOpenWaiting<state>> forwardQueue;
	BDOpenClosed<state, NBSCompareOpenReady<state>, NBSCompareOpenWaiting<state>> backwardQueue;
private:
	double CLowerBound;
};

#endif /* NBSQueue_h */
