#include "base.hh"

class User : public DbModel {
  public:
	double cash;
	User(bsoncxx::document::value doc) : cash{doc.view()["cash"].get_double()} {}
	bsoncxx::document::value to_document() {
		bsoncxx::builder::stream::document builder{};
		return builder << "cash" << cash << bsoncxx::builder::stream::finalize;
	}
};
