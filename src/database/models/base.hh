#include <bsoncxx/json.hpp>

#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>

class Base {
  public:
	virtual bsoncxx::document::value to_document() = 0;
	virtual ~Base() = default;
};
