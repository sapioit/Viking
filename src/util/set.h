#ifndef SET_H
#define SET_H

#include <util/utility.h>

namespace Utility
{

/* The containers are supposed to be sorted */
template <class InputIt1, class InputIt2, class OutputIt, class Compare, class Merge>
OutputIt SetIntersection(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutputIt d_first,
			 Compare comp, Merge merge)
{
	while (first1 != last1 && first2 != last2) {
		if (comp(*first1, *first2))
			++first1;
		else {
			if (!comp(*first2, *first1))
				*d_first++ = merge(*first1++, *first2);
			++first2;
		}
	}
	return d_first;
}
}

#endif // SET_H
