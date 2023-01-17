#include "GraphicsNode.h"

#include <format>
#include <ranges>
#include <string_view>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <functional>

// From https://helloacm.com/convert-a-string-to-camel-case-format-in-c/#:~:text=How%20to%20Convert%20a%20String%20into%20Camel%20Case%20in%20C%2B%2B%3F&text=function,end()%2C%20data.
std::string toCamelCase(const std::string& text) {
	auto rn = text
		| std::ranges::views::split(' ')
		| std::ranges::views::transform([](auto&& str) { return std::string_view(&*str.begin(), std::ranges::distance(str)); });

	std::vector<std::string> words(rn.begin(), rn.end());
	if (words.empty()) return "";

	std::function<std::string(std::string)> stringToLower = [](std::string data) {
		std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c) {
			return std::tolower(c);
			});
		return data;
	};
	std::string ans = stringToLower(words[0]);
	for (size_t i = 1; i < words.size(); ++i) {
		auto w = words[i];
		if (std::all_of(w.begin(), w.end(), ::isupper)) {
			ans += w;
		}
		else {
			ans += (char)std::toupper(w[0]);
			ans += stringToLower(w.substr(1));
		}
	}
	return ans;
}

void GraphicsNode::addParam(const std::string& name, ValueType type) {
	m_params[name] = {
		.value = RawValue(),
		.type = type
	};
}

void GraphicsNode::setup() {
	onCreate();
}

NodeValue GraphicsNode::solve() {
	m_solved = true;
	return NodeValue();
}
