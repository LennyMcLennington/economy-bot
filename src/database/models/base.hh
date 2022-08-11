#include <bsoncxx/json.hpp>

#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>

class DbModel {
  public:
	virtual bsoncxx::document::value to_document() = 0;
	virtual ~DbModel() = default;
};
