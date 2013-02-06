#ifndef INTERPOLATING_MAP
#define INTERPOLATING_MAP

#include <vector>

template<typename type, typename proportionType = float>
struct InterpolatingResult
{
	type* left;
	type* right;
	proportionType proportion;

	InterpolatingResult(type* newLeft, type* newRight, proportionType newProportion) : left(newLeft), right(newRight), proportion(newProportion) {}
};

/** This template helps storing objects which are on a linear scale.
 * Each object is placed on the linear scale, at a specific ordinate.
 * When querying the map with operator[], just pass an ordinate as parameter and the map will tell you the objects between this position and the relative position to the objects.
 * 
 * WARNING : this class does not store objects like STL maps, it stores POINTERS.
 * 
 * Example :
 * {
 *  InterpolatingMap<std::string> map;
 *  // Insert values
 *  map.insert("Hello", 2.3);
 *  map.insert("World", 2.9);
 *  
 *  // Query the map at position 2.5
 *  InterpolatingResult<std::string> result = map[2.5];
 * }
 * 
 * Result :
 * *result.left == "Hello";
 * *result.right == "World";
 * result.proportion = 0.333333;
 */

template<typename type, typename ord = float, typename proportionType = float>
class InterpolatingMap
{
private:
	std::vector<ord> positions;
	std::vector<type*> obj;

public:
	InterpolatingResult<type> operator[](ord position)
	{
		if(positions.size() == 0)
		{
			// Map is empty
			return InterpolatingResult<type, proportionType>(NULL, NULL, (proportionType) 0 );
		}

		if(position < positions[0])
		{
			// Position is before the first element
			return InterpolatingResult<type, proportionType>(NULL, obj[0], (proportionType) 1 );
		}

		if(positions.size() == 1)
		{
			if(position == positions[0])
			{
				// On the first element
				return InterpolatingResult<type, proportionType>(obj[0], obj[0], position);
			}
			else
			{
				// Between the first and the second element
				return InterpolatingResult<type, proportionType>(obj[0], NULL, position);
			}
		}

		unsigned int i=1;
		while(i<positions.size() && positions[i] < position)
			++i;

		if(i == positions.size())
		{
			// After the last element
		}

		if(position == positions[i])
		{
			// Exactly on an element
			return InterpolatingResult<type, proportionType>(obj[i], obj[i], (proportionType) 0 );
		}
		else
		{
			// Between two elements
			return InterpolatingResult<type, proportionType>(obj[i-1], obj[i], (proportionType)(position - positions[i-1]) / (proportionType)(positions[i] - positions[i-1]) );
		}
	}

	typedef typename std::vector<type*>::iterator typeIterator;

	type* getBefore(ord position)
	{
		if(positions.size() == 0 || position < positions[0])
		{
			return NULL;
		}

		int i=0;
		while(i<positions.size() && positions[i] <= position)
			i++;

		return obj[i-1];
	}

	type* getAfter(ord position)
	{
		if(positions.size() == 0 || position >= positions[positions.size() - 1] )
		{
			return NULL;
		}


		int i=0;
		while(i<positions.size() && positions[i] <= position)
			i++;

		return obj[i];
	}

	ord getPrevious(ord position)
	{
		if(positions.size() == 0 || position < positions[0])
		{
			return NULL;
		}

		int i=0;
		while(i<positions.size() && positions[i] <= position)
			i++;

		return positions[i-1];
	}

	ord getNext(ord position)
	{
		if(positions.size() == 0 || position >= positions[positions.size() - 1])
		{
			return NULL;
		}


		int i=0;
		while(i<positions.size() && positions[i] <= position)
			i++;

		return positions[i];
	}

	typeIterator begin()
	{
		return obj.begin();
	}

	typeIterator end()
	{
		return obj.end();
	}

	typeIterator front()
	{
		return obj.front();
	}

	typeIterator back()
	{
		return obj.back();
	}

	typeIterator size()
	{
		return obj.size();
	}

	void push_back(ord newPosition, type* newObj)
	{
		//xassert(positions.size() == 0 || newPosition > positions[positions.size() - 1], "Cannot append before or on the last element. Use insert instead.");
		positions.push_back(newPosition);
		obj.push_back(newObj);
	}

	void pop_back()
	{
		positions.pop_back();
		obj.pop_back();
	}

	void insert(ord newPosition, type* newObj)
	{
		if(positions.size() == 0 || newPosition >= positions[positions.size() - 1])
		{
			push_back(newPosition, newObj);
			return;
		}

		int i=0;
		while(i<positions.size() && positions[i] < newPosition)
			i++;

		// Insert element at newPosition.
		positions.insert( positions.begin() + i, newPosition);
		obj.insert( obj.begin() + i, newObj);
	}
};

#endif /* INTERPOLATING_MAP */

