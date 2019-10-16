cmake_minimum_required(VERSION 3.14)
project(triple_store)

set(CMAKE_CXX_STANDARD 14)

include_directories(/Users/xu/include)
link_directories(/Users/xu/lib)
link_libraries(sdsl divsufsort divsufsort64)
add_executable(triple_store main.cpp TripleStore.cpp TripleStore.hpp RDFStorage.cpp RDFStorage.hpp Dictionary.cpp Dictionary.hpp RankBaseElement.cpp RankBaseElement.hpp Util.hpp RDFTypeStore.cpp RDFTypeStore.hpp Constant.hpp quickSortTS.hpp TriplePattern.cpp TriplePattern.hpp)
