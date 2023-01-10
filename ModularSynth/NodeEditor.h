#pragma once
#ifndef NODEEDITOR_H
#define NODEEDITOR_H

#define NOMINMAX

#include "Control.h"

#include <vector>
#include <array>
#include <string>
#include <memory>
#include <algorithm>

#include "NodeGraph.h"

enum class NodeEditorState {
	idling = 0,
	draggingNode,
	draggingView,
	draggingConnection,
	connecting
};

class VisualNode {
	friend class NodeEditor;
public:
	void onDraw(NVGcontext* ctx, float deltaTime);

	virtual size_t addInput(const std::string& name, NodeValueType type);
	virtual size_t addOutput(const std::string& name, NodeValueType type);

	virtual std::string name() const { return "Node"; };
	virtual Color color() const { return { .r = 0.15f, .g = 0.76f, .b = 0.62f, .a = 1.0f }; };

	virtual NodeValue solve() { return NodeValue(); }

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
	std::vector<NodeValue> m_inputs, m_outputs;

private:
	std::vector<std::string> m_inputNames, m_outputNames;
};

template <typename T>
concept VisualNodeObject = std::is_base_of<VisualNode, T>::value;

struct VisualConnection {
	VisualNode* source;
	VisualNode* destination;
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

	template <VisualNodeObject T>
	T* createNode() {
		T* instance = new T();
		instance->m_id = g_NodeID++;
		m_nodes.push_back(std::unique_ptr<VisualNode>(instance));
		return m_nodes.back().get();
	}

	VisualNode* get(size_t id) {
		auto pos = std::find_if(m_nodes.begin(), m_nodes.end(), [id](const std::unique_ptr<VisualNode>& ob) {
			return ob->id() == id;
		});
		if (pos != m_nodes.end()) {
			return pos->get();
		}
		return nullptr;
	}

	void connect(VisualNode* source, size_t sourceOutput, VisualNode* destination, size_t destinationInput);

private:
	std::vector<std::unique_ptr<VisualNode>> m_nodes;
	std::vector<VisualConnection> m_connections;

	size_t m_selectedNode{ 0 };
	int m_selectedOutput{ -1 };
	NodeEditorState m_state{ NodeEditorState::idling };
	Point m_mousePos{ 0, 0 };

	static size_t g_NodeID;
};

#endif // NODEEDITOR_H
