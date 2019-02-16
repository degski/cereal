
#pragma once

#include <boost/uuid/uuid.hpp>

// #include <boost/uuid/uuid_io.hpp>
// #include <boost/lexical_cast.hpp>

#include <cereal/archives/binary.hpp>

namespace cereal {

	template <class Archive>
	inline void save ( Archive &ar_, const boost::uuids::uuid &uuid_ ) {

		// JSON, XML...

		// std::string val = boost::lexical_cast<std::string> ( uuid_ );
		// ar ( val );

		// Binary...

		ar_ ( cereal::binary_data ( &uuid_, uuid_.static_size ( ) ) );
	}

	template <class Archive>
	inline void load ( Archive &ar_, boost::uuids::uuid& uuid_ ) {

		// JSON, XML...

		// std::string val;
		// ar ( val );
		// uuid_ = boost::lexical_cast<boost::uuids::uuid> ( val );

		// Binary...

		ar_ ( cereal::binary_data ( &uuid_, uuid_.static_size ( ) ) );
	}

} // namespace cereal
