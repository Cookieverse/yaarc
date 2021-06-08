

#ifndef YAARC_COMMAND_HPP
#define YAARC_COMMAND_HPP
namespace yaarc::impl {
	struct Command {
		std::vector<uint8_t> Data;
		std::function<void(std::error_code, Value)> Callback;
		size_t FailCount;

		Command(std::vector<uint8_t> data, std::function<void(std::error_code, Value)> callback,
				size_t failCount = 0) : Data(std::move(data)), Callback(std::move(callback)),
										FailCount(failCount) {}

		Command(Command& other) {
			Data.insert(Data.begin(), other.Data.begin(), other.Data.end());
			Callback = other.Callback;
			FailCount = other.FailCount;
		}

		Command(Command&& other) noexcept {
			Data = std::move(other.Data);
			Callback = std::move(other.Callback);
			FailCount = other.FailCount;
		}

		void Invoke(std::error_code ec, Value result) {
			if (Callback) {
				Callback(std::move(ec), std::move(result));
			}
		}
	};
}
#endif //YAARC_COMMAND_HPP
