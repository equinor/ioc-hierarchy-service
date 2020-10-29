#include "models/models.h"

#include <fstream>

#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/variant.hpp>

int main() {
  auto modelhierarchy = std::vector<NodeType>(
    #include "hierarchy_dump.cpp"
  );
  std::ofstream output("serialized_hierarchy.dat");
  boost::archive::text_oarchive oa(output);
  oa << modelhierarchy;
  output.close();
}
