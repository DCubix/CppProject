#include "NodeGraph.h"

#include <cassert>
#include <stack>

size_t NodeGraph::g_NodeID = 1;

size_t Node::addInput(const std::string& name, ValueType type) {
	m_inputs.push_back({ .type = type });
	m_inputNames.push_back(name);
	return m_inputs.size() - 1;
}

size_t Node::addOutput(const std::string& name, ValueType type) {
	m_outputs.push_back({ .type = type });
	m_outputNames.push_back(name);
	return m_outputs.size() - 1;
}

NodeValue& Node::output(size_t index) {
	return m_outputs[index];
}

NodeValue& Node::input(size_t index) {
	return m_inputs[index];
}

NodeValue* Node::input(const std::string& in) {
	auto pos = std::find(m_inputNames.begin(), m_inputNames.end(), in);
	if (pos == m_inputNames.end()) {
		return nullptr;
	}
	auto dist = std::distance(m_inputNames.begin(), pos);
	return &m_inputs[dist];
}


void NodeGraph::connect(Node* source, size_t sourceOutput, Node* destination, size_t destinationInput) {
	Connection conn{
		.source = source,
		.destination = destination,
		.destinationInput = destinationInput,
		.sourceOutput = sourceOutput
	};
	source->m_outputs[sourceOutput].connected = true;
	destination->m_inputs[destinationInput].connected = true;
	m_connections.push_back(conn);
	buildNodePath();
}

void NodeGraph::solve() {
	if (m_nodePath.empty()) buildNodePath();

	std::stack<Node*> nodes;
	for (size_t i = 0; i < m_nodePath.size(); i++) {
		Node* node = get(m_nodePath[i]);
		node->m_solved = false;
		node->m_changed = false;
		nodes.push(node);
	}

	// solve nodes
	while (!nodes.empty()) {
		Node* node = nodes.top(); nodes.pop();
		if (!node->m_solved) {
			node->solve();
		}

		// set values
		for (auto conn : getNodeOutputConnections(node)) {
			Node* from = conn.source;
			Node* to = conn.destination;
			to->input(conn.destinationInput).value = from->output(conn.sourceOutput).value;
		}
	}
}

bool NodeGraph::hasChanges() const {
	for (auto&& node : m_nodes) {
		if (node->changed()) return true;
	}
	return false;
}

std::vector<Connection> NodeGraph::getNodeInputConnections(Node* node) {
	std::vector<Connection> ret;
	for (auto&& conn : m_connections) {
		if (conn.destination == node) {
			ret.push_back(conn);
		}
	}
	return ret;
}

std::vector<Connection> NodeGraph::getNodeOutputConnections(Node* node) {
	std::vector<Connection> ret;
	for (auto&& conn : m_connections) {
		if (conn.source == node) {
			ret.push_back(conn);
		}
	}
	return ret;
}

std::vector<Connection> NodeGraph::getConnectionsToInput(Node* node, size_t input) {
	std::vector<Connection> ret;
	for (auto&& conn : m_connections) {
		if (conn.destination == node && conn.destinationInput == input) {
			ret.push_back(conn);
		}
	}
	return ret;
}

std::vector<size_t> NodeGraph::getPathFrom(Node* node) {
	assert(node != nullptr);

	std::vector<size_t> path;
	
	path.push_back(node->id());
	for (auto conn : getNodeInputConnections(node)) {
		auto retPath = getPathFrom(conn.source);
		path.insert(path.end(), retPath.begin(), retPath.end());
	}

	return path;
}

std::vector<size_t> NodeGraph::getRightMostNodes() {
	std::vector<size_t> ret;
	for (auto&& node : m_nodes) {
		Node* nodePtr = node.get();
		for (size_t i = 0; i < node->m_outputs.size(); i++) {
			auto connections = getNodeOutputConnections(nodePtr);
			if (connections.empty()) {
				ret.push_back(node->id());
			}
		}
	}
	return ret;
}

void NodeGraph::buildNodePath() {
	m_nodePath.clear();
	for (size_t nid : getRightMostNodes()) {
		auto path = getPathFrom(get(nid));
		for (size_t pid : path) {
			if (std::find(m_nodePath.begin(), m_nodePath.end(), pid) != m_nodePath.end()) continue;
			m_nodePath.push_back(pid);
		}
	}
}
