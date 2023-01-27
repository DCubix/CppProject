#include "NodeGraph.h"

#include <cassert>
#include <stack>
#include <queue>

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

NodeValue& Node::texture(size_t index) {
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

void NodeGraph::removeConnection(Node* source, size_t sourceOutput, Node* destination, size_t destinationInput) {
	auto pos = std::find_if(m_connections.begin(), m_connections.end(), [=](const Connection& cn) {
		return cn.destination == destination &&
			cn.destinationInput == destinationInput &&
			cn.source == source &&
			cn.sourceOutput == sourceOutput;
	});
	if (pos == m_connections.end()) return;
	source->m_outputs[sourceOutput].connected = false;
	destination->m_inputs[destinationInput].connected = false;
	m_connections.erase(pos);
	buildNodePath();
}

void NodeGraph::solve() {
	if (m_nodePath.empty()) buildNodePath();

	std::stack<Node*> nodes;
	for (size_t i = m_nodePath.size(); i-- > 0;) {
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
			to->input(conn.destinationInput).value = from->texture(conn.sourceOutput).value;
		}
	}
}

bool NodeGraph::hasChanges() const {
	for (auto&& node : m_nodes) {
		if (node->changed()) return true;
	}
	return false;
}

void NodeGraph::clearChanges() {
	for (auto&& node : m_nodes) {
		if (node->changed()) node->m_changed = false;
	}
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
	for (auto conn : getNodeOutputConnections(node)) {
		path.push_back(conn.destination->id());
	}
	return path;
}

std::vector<size_t> NodeGraph::getLeftMostNodes() {
	std::vector<size_t> ret;
	for (auto&& node : m_nodes) {
		Node* nodePtr = node.get();
		auto connections = getNodeInputConnections(nodePtr);
		if (connections.empty()) {
			ret.push_back(node->id());
		}
	}
	return ret;
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

#ifdef _DEBUG
#include <iostream>
#endif

void NodeGraph::buildNodePath() {
#define nodeExistsVec(vec, nid) (std::find(vec.begin(), vec.end(), (nid)) != vec.end())
#define nodeExists(nid) nodeExistsVec(m_nodePath, nid)
	m_nodePath.clear();

	std::vector<size_t> nodeQueue;
	for (size_t nid : getLeftMostNodes()) {
		nodeQueue.push_back(nid);
	}

	while (!nodeQueue.empty()) {
		size_t pid = nodeQueue.front();
		if (nodeExists(pid)) {
			nodeQueue.erase(nodeQueue.begin());
			continue;
		}

		auto path = getPathFrom(get(pid));
		for (size_t nid : path) nodeQueue.push_back(nid);

		// are all the inputs processed?
		bool ok = true;
		for (const auto& conn : getNodeInputConnections(get(pid))) {
			if (!nodeExists(conn.source->id())) {
				ok = false;
				break;
			}
		}

		if (ok) {
			m_nodePath.push_back(pid);
			nodeQueue.erase(nodeQueue.begin());
		}
		else {
			nodeQueue.erase(nodeQueue.begin());
			nodeQueue.insert(nodeQueue.end(), pid);
		}
	}

#ifdef _DEBUG
	std::cout << "[ ";
	size_t i = 0;
	for (size_t pid : m_nodePath) {
		std::cout << pid;
		if (i < m_nodePath.size() - 1) {
			std::cout << ", ";
		}
		i++;
	}
	std::cout << "]\n";
#endif
}
