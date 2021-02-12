#ifndef UT_STATS_STATS_SUT_CPP
#define UT_STATS_STATS_SUT_CPP

#include "stats.hpp"

namespace stats
{
class stats_sut : public stats
{
public:
    stats_sut(boost::asio::io_context& io_ctx, const int print_period,
              const std::string& output_file_name, const std::vector<std::string>& msg_names)
        : stats(io_ctx, print_period, output_file_name, msg_names){};

    const snapshot& get_total_snap() const { return total_snap; }

    const snapshot& get_partial_snap() const { return partial_snap; }

    const std::map<std::string, snapshot>& get_msg_snaps() const { return msg_snaps; }
};
}  // namespace stats
#endif