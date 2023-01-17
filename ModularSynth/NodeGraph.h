#pragma once

#include <array>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>

using RawValue = std::array<float, 4>;

enum class ValueType : size_t {
	none = 0,
	scalar,
	vec2,
	vec3,
	vec4,
	image
};

struct NodeValue {
	RawValue value{ 0.0f };
	ValueType type;
	bool connected{ false };
};

class Node {
	friend class NodeGraph;
public:
	virtual ~Node() = default;

	size_t addInput(const std::string& name, ValueType type);
	size_t addOutput(const std::string& name, ValueType type);

	NodeValue& output(size_t index);
	NodeValue& input(size_t index);
	NodeValue* input(const std::string& in);

	virtual NodeValue solve() = 0;
	virtual void setup() = 0;

	size_t id() const { return m_id; }
	size_t outputCount() const { return m_outputs.size(); }
	size_t inputCount() const { return m_inputs.size(); }

	bool hasInput(const std::string& in) { return std::find(m_inputNames.begin(), m_inputNames.end(), in) != m_inputNames.end(); }
	bool hasOutput(const std::string& out) { return std::find(m_outputNames.begin(), m_outputNames.end(), out) != m_outputNames.end(); }

	const std::string& inputName(size_t index) const { return m_inputNames[index]; }
	const std::string& outputName(size_t index) const { return m_outputNames[index]; }

	size_t inputIndex(const std::string& in) {
		auto pos = std::find(m_inputNames.begin(), m_inputNames.end(), in);
		return std::distance(m_inputNames.begin(), pos);
	}

	bool changed() const { return m_changed; }

protected:
	bool m_solved{ false };
	bool m_changed{ false };

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
	void removeConnection(Node* source, size_t sourceOutput, Node* destination, size_t destinationInput);

	virtual void solve();
	size_t lastNode() const { return m_nodePath.empty() ? 0 : m_nodePath.front(); }

	bool hasChanges() const;
	void clearChanges();

protected:
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

	std::vector<Connection> getConnectionsToInput(Node* node, size_t input);

	std::vector<Connection> getNodeInputConnections(Node* node);
	std::vector<Connection> getNodeOutputConnections(Node* node);
	std::vector<size_t> getRightMostNodes();
	std::vector<size_t> getPathFrom(Node* node);
	void buildNodePath();

	static size_t g_NodeID;
};
