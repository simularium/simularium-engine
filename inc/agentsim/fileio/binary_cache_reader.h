#ifndef AICS_BINARY_CACHE_READER_H
#define AICS_BINARY_CACHE_READER_H

#include "agentsim/agent_data.h"
#include <string>
#include <fstream>
#include <iostream>

namespace aics {
namespace agentsim {
namespace fileio {

class BinaryCacheReader {
public:
    bool DeserializeFrame(
      std::ifstream&is,
      std::size_t frameNumber,
      AgentDataFrame& adf
    );

private:
    bool GotoFrameNumber(
      std::ifstream& is,
      std::size_t frameNumber
    );

    bool DeserializeAgent(
      std::ifstream& is,
      AgentData& ad
    );

    std::string m_delimiter = "/";
};

} // namespace fileio
} // namespace agentsim
} // namespace aics

#endif // AICS_BINARY_CACHE_READER_H