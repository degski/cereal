
#pragma once

#include <cereal/archives/binary.hpp>


namespace cereal {

	template <class Archive, class StaticClass>
	inline void save ( Archive & ar_, const StaticClass & sc_ ) {
	
		// Binary...

		ar_ ( cereal::binary_data ( & sc_, sizeof ( StaticClass ) ) );
	}

	template <class Archive, class StaticClass>
	inline void load ( Archive & ar_, StaticClass & sc_ ) {
	
		// Binary...

		ar_ ( cereal::binary_data ( & sc_, sizeof ( StaticClass ) ) );
	}

} // namespace cereal
