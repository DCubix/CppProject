#pragma once

#include <array>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

using RawNodeValue = std::array<float, 4>;

enum class NodeValueType : size_t {
	float1 = 1,
	float2,
	float3,
	float4,
	image
};

struct NodeValue {
	RawNodeValue value{ 0.0f };
	NodeValueType type;
};

class Node {
	friend class NodeGraph;
public:
	virtual ~Node() = default;

	size_t addInput(const std::string& name, NodeValueType type);
	size_t addOutput(const std::string& name, NodeValueType type);

	NodeValue& output(size_t index);
	NodeValue& input(size_t index);

	virtual NodeValue solve() = 0;
	virtual void setup() = 0;

	size_t id() const { return m_id; }

protected:
	size_t m_id{ 0 };
	std::vector<NodeValue> m_inputs, m_outputs;
	std::vector<std::string> m_inputNames, m_outputNames;
};

template <typename T>
concept NodeObject = std::is_base_of<Node, T>::value;

struct Connection {
	Node* source;
	Node* destination;
	size_t destinationInput;
	size_t sourceOutput;
};

class NodeGraph {
public:
	template <NodeObject T>
	T* create() {
		T* instance = new T();
		instance->m_id = g_NodeID++;
		m_nodes.push_back(std::unique_ptr<Node>(instance));
		m_nodes.back()->setup();
		return dynamic_cast<T*>(m_nodes.back().get());
	}

	Node* get(size_t id) {
		auto pos = std::find_if(m_nodes.begin(), m_nodes.end(), [id](const std::unique_ptr<Node>& ob) {
			return ob->id() == id;
		});
		if (pos != m_nodes.end()) {
			return pos->get();
		}
		return nullptr;
	}

	void connect(Node* source, size_t sourceOutput, Node* destination, size_t destinationInput);

	void solve();

private:
	std::vector<std::unique_ptr<Node>> m_nodes;
	std::vector<Connection> m_connections;

	/*
	 * SOLVING A NODE GRAPH FROM LEFT TO RIGHT
	 * ====================================================
	 * 1. Get all the input nodes (left-most nodes with no input connections)
	 * 2. For each input node found:
	 *		a. Solve the node (run the behavior code)
	 *		b. For each connection on each output, set the next node inputs accordingly
	 *		c. Solve the next node
	 */

	std::vector<size_t> m_nodePath;

	std::vector<Connection> getNodeInputConnections(Node* node);
	std::vector<Connection> getNodeOutputConnections(Node* node);
	std::vector<size_t> getPathFrom(Node* node);
	void buildNodePath();

	static size_t g_NodeID;
};
