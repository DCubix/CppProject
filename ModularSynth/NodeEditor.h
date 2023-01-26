#pragma once

#define NOMINMAX

#include "Control.h"

#include <vector>
#include <array>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>

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

	const std::string& name() const { return m_name; }
	const Color& color() const { return m_color; }
	const std::string& code() const { return m_code; }

	Rect getOutputRect(size_t index);
	Rect getInputRect(size_t index);
	Dimension computeSize(NVGcontext* ctx);
	const Dimension& size() const { return m_size; }

	virtual Dimension extraSize() { return { 0, 0 }; }
	virtual void onExtraDraw(NVGcontext* ctx, float deltaTime) {}

	size_t outputCount() const { return m_node->outputCount(); }
	size_t inputCount() const { return m_node->inputCount(); }

	size_t id() const { return m_id; }
	Node* node() { return m_node; }

	Point position{ 0, 0 };

protected:
	size_t m_id{ 0 };

	Dimension m_size{ 0, 0 };
	std::vector<Rect> m_outputRects, m_inputRects;
	Color m_color{ 1.0f, 1.0f, 1.0f, 1.0f };
	std::string m_name{ "Node" }, m_code{ "NOD" };

	Node* m_node{ nullptr };
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
	NodeEditor(NodeGraph* graph = nullptr);

	void onDraw(NVGcontext* ctx, float deltaTime) override;

	void onMouseDown(int button, int x, int y) override;
	void onMouseUp(int button, int x, int y) override;
	void onMouseMove(int x, int y, int dx, int dy) override;
	void onMouseLeave() override;

	void onKeyPress(int key) override;
	void onKeyRelease(int key) override;

	template <NodeObject U, VisualNodeObject T = VisualNode>
	T* create(const std::string& name, const std::string& code, Color color) {
		T* instance = new T();
		U* node = m_graph->create<U>();

		instance->m_id = g_NodeID++;
		instance->m_node = node;
		instance->m_name = name;
		instance->m_color = color;
		instance->m_code = code;

		node->solve();

		m_nodes.push_back(std::unique_ptr<T>(instance));

		rebuildDrawOrder();

		return dynamic_cast<T*>(m_nodes.back().get());
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

	VisualNode* getFromOriginalNodeId(size_t id) {
		auto pos = std::find_if(m_nodes.begin(), m_nodes.end(), [id](const std::unique_ptr<VisualNode>& ob) {
			return ob->node()->id() == id;
			});
		if (pos != m_nodes.end()) {
			return pos->get();
		}
		return nullptr;
	}

	void connect(VisualNode* source, size_t sourceOutput, VisualNode* destination, size_t destinationInput);
	void removeConnection(VisualNode* source, size_t sourceOutput, VisualNode* destination, size_t destinationInput);

	NodeGraph* graph() { return m_graph.get(); }

	std::function<void(VisualNode*)> onSelect{ nullptr };
	std::function<void()> onParamChange{ nullptr };

private:
	std::vector<std::unique_ptr<VisualNode>> m_nodes;
	std::vector<VisualConnection> m_connections;

	std::vector<size_t> m_drawOrders;

	std::unique_ptr<NodeGraph> m_graph;

	size_t m_selectedNode{ 0 };
	int m_selectedOutput{ -1 };
	NodeEditorState m_state{ NodeEditorState::idling };
	Point m_mousePos{ 0, 0 };

	void rebuildDrawOrder();

	std::vector<VisualConnection> getConnectionsTo(VisualNode* node);
	std::vector<VisualConnection> getConnectionsFrom(VisualNode* node);
	void moveNodeTree(VisualNode* node, int dx, int dy);

	bool m_shiftPressed{ false };

	static size_t g_NodeID;
};
