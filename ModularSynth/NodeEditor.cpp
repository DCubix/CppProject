#include "NodeEditor.h"

size_t NodeEditor::g_NodeID = 1;

constexpr float titleFontSize = 15.0f;
constexpr float bodyTextFontSize = 12.0f;
constexpr int gapBetweenSides = 32;
constexpr int gapBetweenInOuts = 4;
constexpr int padding = 7;

static Point lerpPoint(Point a, Point b, float t) {
	return {
		.x = int((1.0f - t) * a.x + b.x * t),
		.y = int((1.0f - t) * a.y + b.y * t)
	};
}

static void beginConnection(NVGcontext* ctx, Point a, Point b) {
	const Point ia{ a.x + 20, a.y };
	const Point ib{ b.x - 20, b.y };
	const Point mid{ (a.x + b.x) / 2, (a.y + b.y) / 2 };

	nvgBeginPath(ctx);
	if (ia.x > ib.x) {
		nvgMoveTo(ctx, a.x, a.y);
		nvgBezierTo(ctx, ia.x, ia.y, ia.x, mid.y, mid.x, mid.y);
		nvgBezierTo(ctx, ib.x, mid.y, ib.x, ib.y, b.x, b.y);
	}
	else {
		nvgMoveTo(ctx, a.x, a.y);
		nvgBezierTo(ctx, mid.x, a.y, mid.x, b.y, b.x, b.y);
	}
}

void NodeEditor::onDraw(NVGcontext* ctx, float deltaTime) {
	Rect b = bounds;

	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, 0.0f, 0.0f, b.width, b.height, 16.0f);
	nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.35f));
	nvgFill(ctx);

	nvgSave(ctx);

	nvgScissor(ctx, 6.0f, 6.0f, b.width - 12.0f, b.height - 12.0f);
	for (auto&& node : m_nodes) {
		node->onDraw(ctx, deltaTime);
	}

	nvgSave(ctx);
	nvgStrokeWidth(ctx, 3.0f);
	nvgStrokeColor(ctx, nvgRGBf(1.0f, 1.0f, 1.0f));

	for (auto&& conn : m_connections) {
		Rect outRect = conn.source->getOutputRect(conn.sourceOutput);
		Rect inRect = conn.destination->getInputRect(conn.destinationInput);

		beginConnection(ctx, { outRect.x + 5, outRect.y + 5 }, { inRect.x + 5, inRect.y + 5 });
		nvgStroke(ctx);

		nvgBeginPath(ctx);
		nvgCircle(ctx, outRect.x + 5, outRect.y + 5, 4.5f);
		nvgCircle(ctx, inRect.x + 5, inRect.y + 5, 4.5f);
		nvgFillColor(ctx, nvgRGBf(1.0f, 1.0f, 1.0f));
		nvgFill(ctx);
	}
	nvgRestore(ctx);

	if (m_state == NodeEditorState::draggingConnection) {
		VisualNode* node = get(m_selectedNode);
		if (node) {
			Rect outRect = node->getOutputRect(m_selectedOutput);

			nvgSave(ctx);
			beginConnection(ctx, { outRect.x + 5, outRect.y + 5 }, m_mousePos);
			nvgStrokeWidth(ctx, 3.0f);
			nvgStrokeColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f));
			nvgStroke(ctx);

			nvgBeginPath(ctx);
			nvgCircle(ctx, outRect.x + 5, outRect.y + 5, 4.5f);
			nvgCircle(ctx, m_mousePos.x, m_mousePos.y, 4.5f);
			nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 0.5f));
			nvgFill(ctx);

			nvgRestore(ctx);
		}
	}

	nvgRestore(ctx);
}

void NodeEditor::onMouseDown(int button, int x, int y) {
	const float titleHeight = titleFontSize + 8.0f;

	if (button == 1) {
		bool clickedOnSomet = false;
		for (auto&& node : m_nodes) {
			Dimension sz = node->size();
			Rect bounds = { node->position.x - 5, node->position.y, sz.width + 10, sz.height };
			if (bounds.hasPoint({ x, y })) {
				m_selectedNode = node->id();
				m_selectedOutput = -1;
				clickedOnSomet = true;

				// check for out click
				for (size_t i = 0; i < node->outputCount(); i++) {
					Rect outRect = node->getOutputRect(i);
					outRect.inflate(2);
					if (outRect.hasPoint({ x, y })) {
						m_selectedOutput = i;
						break;
					}
				}

				break;
			}
		}

		if (!clickedOnSomet) {
			m_selectedNode = 0;
		}
		else {
			m_state = m_selectedOutput != -1 ? NodeEditorState::draggingConnection : NodeEditorState::draggingNode;
		}
	}
}

void NodeEditor::onMouseUp(int button, int x, int y) {
	if (m_state == NodeEditorState::draggingConnection) {
		for (auto&& node : m_nodes) {
			for (size_t i = 0; i < node->inputCount(); i++) {
				Rect outRect = node->getInputRect(i);
				outRect.inflate(2);
				if (outRect.hasPoint({ x, y })) {
					VisualNode* source = get(m_selectedNode);
					if (source != node.get()) { // Don't allow self-connection
						connect(source, m_selectedOutput, node.get(), i);
					}
					break;
				}
			}
		}
	}
	m_state = NodeEditorState::idling;
}

void NodeEditor::onMouseMove(int x, int y, int dx, int dy) {
	m_mousePos.x = x;
	m_mousePos.y = y;

	if (m_state == NodeEditorState::draggingNode) {
		VisualNode* node = get(m_selectedNode);
		if (!node) return;

		node->position.x += dx;
		node->position.y += dy;
	}
}

void NodeEditor::onMouseLeave() {
	m_state = NodeEditorState::idling;
}

Rect VisualNode::getOutputRect(size_t index) {
	return m_outputRects[index];
}

Rect VisualNode::getInputRect(size_t index) {
	return m_inputRects[index];
}

void VisualNode::onDraw(NVGcontext* ctx, float deltaTime) {
	Dimension sz = computeSize(ctx);
	Rect b = { 0, 0, sz.width, sz.height };
	Color col = color();

	const float titleHeight = titleFontSize + 8.0f;

	nvgSave(ctx);
	nvgTranslate(ctx, position.x, position.y);

	// drop shadow :)
	NVGpaint shadowPaint = nvgBoxGradient(
		ctx,
		b.x, b.y + 2, b.width, b.height,
		5.0f, 10.0f,
		nvgRGBAf(0.0f, 0.0f, 0.0f, 0.4f),
		nvgRGBAf(0.0f, 0.0f, 0.0f, 0.0f)
	);
	nvgBeginPath(ctx);
	nvgRect(ctx, b.x - 10, b.y - 10, b.width + 20, b.height + 20);
	nvgRoundedRect(ctx, b.x, b.y, b.width, b.height, 5.0f);
	nvgPathWinding(ctx, NVG_HOLE);
	nvgFillPaint(ctx, shadowPaint);
	nvgFill(ctx);

	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, 0.0f, 0.0f, b.width, b.height, 5.0f);
	nvgFillColor(ctx, nvgRGBAf(col.r, col.g, col.b, col.a));
	nvgFill(ctx);

	nvgBeginPath(ctx);
	nvgRoundedRectVarying(ctx, 0.0f, 0.0f, b.width, titleHeight, 5.0f, 5.0f, 0.0f, 0.0f);
	nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.5f));
	nvgFill(ctx);

	nvgFillColor(ctx, nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f));
	nvgFontSize(ctx, titleFontSize);
	nvgTextAlign(ctx, NVG_ALIGN_MIDDLE);
	nvgText(ctx, padding, titleHeight / 2 + 1.5f, name().c_str(), nullptr);

	float size[4];
	float posY = titleHeight + padding;

	if (m_inputRects.size() < m_inputs.size()) {
		m_inputRects.resize(m_inputs.size());
	}

	nvgFontSize(ctx, bodyTextFontSize);
	nvgFontFace(ctx, "default-bold");
	nvgTextAlign(ctx, NVG_ALIGN_TOP);
	nvgStrokeWidth(ctx, 1.0f);

	for (size_t i = 0; i < m_inputs.size(); i++) {
		nvgTextBounds(ctx, 0.0f, 0.0f, m_inputNames[i].c_str(), nullptr, size);
		nvgFillColor(ctx, nvgRGBf(0.0f, 0.0f, 0.0f));
		nvgText(ctx, padding, posY + 1.5f, m_inputNames[i].c_str(), nullptr);

		float halfTextHeight = (size[3] - size[1]) / 2.0f;

		nvgBeginPath(ctx);
		nvgCircle(ctx, 0.0f, posY + halfTextHeight, 5.0f);
		nvgStrokeColor(ctx, nvgRGB(0, 0, 0));
		nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.8f));
		nvgFill(ctx);
		nvgStroke(ctx);

		Rect outRect = { position.x - 5.0f, posY + halfTextHeight - 5.0f + position.y, 10.0f, 10.0f };
		m_inputRects[i] = outRect;

		posY += halfTextHeight * 2.0f + gapBetweenInOuts;
	}

	nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);

	posY = titleHeight + padding;

	if (m_outputRects.size() < m_outputs.size()) {
		m_outputRects.resize(m_outputs.size());
	}

	for (size_t i = 0; i < m_outputs.size(); i++) {
		nvgTextBounds(ctx, 0.0f, 0.0f, m_outputNames[i].c_str(), nullptr, size);
		nvgFillColor(ctx, nvgRGBf(0.0f, 0.0f, 0.0f));
		nvgText(ctx, b.width - padding, posY + 1.5f, m_outputNames[i].c_str(), nullptr);

		float halfTextHeight = (size[3] - size[1]) / 2.0f;

		nvgBeginPath(ctx);
		nvgCircle(ctx, b.width, posY + halfTextHeight, 5.0f);
		nvgStrokeColor(ctx, nvgRGB(0, 0, 0));
		nvgFillColor(ctx, nvgRGBAf(0.0f, 0.0f, 0.0f, 0.8f));
		nvgFill(ctx);
		nvgStroke(ctx);

		Rect outRect = { b.width - 5.0f + position.x, posY + halfTextHeight - 5.0f + position.y, 10.0f, 10.0f };
		m_outputRects[i] = outRect;

		posY += halfTextHeight * 2.0f + gapBetweenInOuts;
	}

	nvgRestore(ctx);
}

size_t VisualNode::addInput(const std::string& name, NodeValueType type) {
	m_inputs.push_back({ .type = type });
	m_inputNames.push_back(name);
	return m_inputs.size() - 1;
}

size_t VisualNode::addOutput(const std::string& name, NodeValueType type) {
	m_outputs.push_back({ .type = type });
	m_outputNames.push_back(name);
	return m_outputs.size() - 1;
}

Dimension VisualNode::computeSize(NVGcontext* ctx) {
	int width = padding * 2;
	int height = padding * 2;

	nvgSave(ctx);

	// title size
	float size[4];

	nvgFontSize(ctx, titleFontSize);
	nvgFontFace(ctx, "default-bold");
	nvgTextBounds(ctx, 0.0f, 0.0f, name().c_str(), nullptr, size);

	width += int(size[2] - size[0]);
	height += int(size[3] - size[1]);

	// inputs text size (get the max)
	nvgFontSize(ctx, bodyTextFontSize);
	nvgFontFace(ctx, "default-bold");

	float maxInputWidth = 0.0f;
	float inputsHeight = 0.0f;

	for (size_t i = 0; i < m_inputs.size(); i++) {
		nvgTextBounds(ctx, 0.0f, 0.0f, m_inputNames[i].c_str(), nullptr, size);
		maxInputWidth = std::max(maxInputWidth, size[2] - size[0]);
		inputsHeight += size[3] - size[1];
	}

	// outputs text size (get the max)
	float maxOutputWidth = 0.0f;
	float outputsHeight = 0.0f;

	for (size_t i = 0; i < m_outputs.size(); i++) {
		nvgTextBounds(ctx, 0.0f, 0.0f, m_outputNames[i].c_str(), nullptr, size);
		maxOutputWidth = std::max(maxOutputWidth, size[2] - size[0]);
		outputsHeight += size[3] - size[1];
	}

	width = std::max(width, int(maxInputWidth) + gapBetweenSides + int(maxOutputWidth));
	height += std::max(int(inputsHeight), int(outputsHeight));

	const size_t connectableCount = std::max(m_inputs.size(), m_outputs.size());
	height += gapBetweenInOuts * connectableCount;

	// some extra magic-number-driven padding on the bottom...
	height += 8;

	nvgRestore(ctx);

	m_size.width = width;
	m_size.height = height;

	return m_size;
}

void NodeEditor::connect(VisualNode* source, size_t sourceOutput, VisualNode* destination, size_t destinationInput) {
	VisualConnection conn{
		.source = source,
		.destination = destination,
		.destinationInput = destinationInput,
		.sourceOutput = sourceOutput
	};
	m_connections.push_back(conn);
}
