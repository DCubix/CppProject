#pragma once

#define NOMINMAX

#include "Control.h"

#include <vector>
#include <array>
#include <string>
#include <memory>
#include <algorithm>

enum NodeValueType {
	float1 = 1,
	float2,
	float3,
	float4
};

enum class NodeEditorState {
	idling = 0,
	draggingNode,
	draggingView,
	draggingConnection,
	connecting
};

struct NodeValue {
	std::array<float, 4> value{ 0.0f };
	NodeValueType type;
};

class Node {
	friend class NodeEditor;
public:
	void onDraw(NVGcontext* ctx, float deltaTime);
	
	virtual size_t addInput(const std::string& name, NodeValueType type);
	virtual size_t addOutput(const std::string& name, NodeValueType type);

	virtual std::string name() const { return "Node"; };
	virtual Color color() const { return { .r = 0.15f, .g = 0.76f, .b = 0.62f, .a = 1.0f }; };

	Rect getOutputRect(size_t index);
	Rect getInputRect(size_t index);
	Dimension computeSize(NVGcontext* ctx);
	const Dimension& size() const { return m_size; }

	size_t outputCount() const { return m_outputs.size(); }
	size_t inputCount() const { return m_inputs.size(); }

	size_t id() const { return m_id; }

	Point position{ 0, 0 };

protected:
	size_t m_id{ 0 };
	Dimension m_size{ 0, 0 };
	std::vector<Rect> m_outputRects, m_inputRects;

private:
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

class NodeEditor : public Control {
public:
	void onDraw(NVGcontext* ctx, float deltaTime) override;

	void onMouseDown(int button, int x, int y) override;
	void onMouseUp(int button, int x, int y) override;
	void onMouseMove(int x, int y, int dx, int dy) override;
	void onMouseLeave() override;

	template <NodeObject T>
	T* createNode() {
		T* instance = new T();
		instance->m_id = g_NodeID++;
		m_nodes.push_back(std::unique_ptr<Node>(instance));
		return m_nodes.back().get();
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

private:
	std::vector<std::unique_ptr<Node>> m_nodes;
	std::vector<Connection> m_connections;

	size_t m_selectedNode{ 0 };
	int m_selectedOutput{ -1 };
	NodeEditorState m_state{ NodeEditorState::idling };
	Point m_mousePos{ 0, 0 };

	static size_t g_NodeID;
};

