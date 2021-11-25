#include "base.hh"

class User : Base {
  public:
	int cash;
	bsoncxx::document::value to_document() {
		bsoncxx::builder::stream::document builder{};
		return builder << "cash" << cash << bsoncxx::builder::stream::finalize;
	}
};
