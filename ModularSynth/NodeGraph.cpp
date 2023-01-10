#include "NodeGraph.h"

#include <cassert>

size_t NodeGraph::g_NodeID = 1;

size_t Node::addInput(const std::string& name, NodeValueType type) {
	m_inputs.push_back({ .type = type });
	m_inputNames.push_back(name);
	return m_inputs.size() - 1;
}

size_t Node::addOutput(const std::string& name, NodeValueType type) {
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


void NodeGraph::connect(Node* source, size_t sourceOutput, Node* destination, size_t destinationInput) {
	Connection conn{
		.source = source,
		.destination = destination,
		.destinationInput = destinationInput,
		.sourceOutput = sourceOutput
	};
	m_connections.push_back(conn);
	buildNodePath();
}

void NodeGraph::solve() {

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

std::vector<size_t> NodeGraph::getPathFrom(Node* node) {
	assert(node != nullptr);

	std::vector<size_t> path;
	
	path.push_back(node->id());
	for (auto conn : getNodeOutputConnections(node)) {
		auto retPath = getPathFrom(conn.destination);
		path.insert(path.end(), retPath.begin(), retPath.end());
	}

	return path;
}

void NodeGraph::buildNodePath() {
	m_nodePath.clear();
	for (auto&& node : m_nodes) {
		Node* nodePtr = node.get();
		auto inConnections = getNodeInputConnections(nodePtr);
		if (inConnections.empty()) {
			auto path = getPathFrom(nodePtr);
			m_nodePath.insert(path.end(), path.begin(), path.end());
		}
	}
}
