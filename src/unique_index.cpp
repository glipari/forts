#include "unique_index.hpp"

int UniqueIndex::index = 0;

int UniqueIndex::get_next_index()
{
	return index++;
}
