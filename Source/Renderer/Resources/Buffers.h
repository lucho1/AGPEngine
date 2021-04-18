#ifndef _BUFFERS_H_
#define _BUFFERS_H_

#include "Core/Globals.h"
#include "Renderer/Utils/RendererUtils.h"

using namespace RendererUtils;

// ----- Element of the Buffer -----
struct BufferElement
{
	// --- Variables ---
	std::string Name = "unnamed";
	SHADER_DATA Type = SHADER_DATA::NONE;
	size_t Offset = 0;
	uint Size = 0;
	bool Normalized = false;

	// --- Functions ---
	BufferElement() = default;
	BufferElement(SHADER_DATA type, const std::string& name, bool normalized = false)
		: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Normalized(normalized) {}

	uint GetElementTypeCount() const { return ShaderDataTypeCount(Type); }
};



// ----- Definition of the Layout of the Buffer -----
class BufferLayout
{
public:

	// --- Des/Construction ---
	BufferLayout(std::initializer_list<BufferElement> elements) : m_Elements(elements) { CalculateOffsetAndStride(); }
	BufferLayout() = default;

	// --- Getters ---
	inline const std::vector<BufferElement>& GetElements()	const { return m_Elements; }
	inline const uint GetStride()							const { return m_Stride; }

	// --- Iterators ---
	std::vector<BufferElement>::const_iterator begin()		const { return m_Elements.begin(); }
	std::vector<BufferElement>::const_iterator end()		const { return m_Elements.end(); }

private:

	// --- Private Methods ---
	void CalculateOffsetAndStride()
	{
		size_t offset = 0;
		m_Stride = 0;
		for (auto& element : m_Elements)
		{
			element.Offset = offset;
			offset += element.Size;
			m_Stride += element.Size;
		}
	}

private:

	// --- Variables ---
	std::vector<BufferElement> m_Elements;
	uint m_Stride = 0;
};



// ------------------------------------------------------------------------------
// ----- Vertex Buffer -----
class VertexBuffer
{
public:

	// --- Des/Construction ---
	VertexBuffer(float* vertices, uint size);
	VertexBuffer(uint size);
	~VertexBuffer();
	
	// --- Class Methods ---
	void Bind() const;
	void Unbind() const;

	// --- Getters/Setters ---
	const BufferLayout& GetLayout()					const { return m_Layout; }
	void SetLayout(const BufferLayout& layout)		{ m_Layout = layout; }
	void SetData(const void* data, uint size);

private:

	// --- Variables ---
	uint m_ID = 0;
	BufferLayout m_Layout;
};



// ---- Index Buffer ----
class IndexBuffer
{
public:

	// --- Des/Construction ---
	IndexBuffer(uint* vertices, uint count);
	~IndexBuffer();

	// --- Class Methods ---
	void Bind() const;
	void Unbind() const;

	// -- Getters --
	uint GetCount() const { return m_Count; }

private:

	// --- Variables ---
	uint m_ID = 0, m_Count = 0;
};



// ---- Vertex Array ----
class VertexArray
{
public:

	// --- Des/Construction ---
	VertexArray();
	~VertexArray();

	// --- Class Methods ---
	void Bind() const;
	void Unbind() const;

	void SetIndexBuffer(const Ref<IndexBuffer>& index_buffer);
	void AddVertexBuffer(const Ref<VertexBuffer>& vertex_buffer);

	// --- Getters ---
	inline const Ref<IndexBuffer>& GetIndexBuffer()					const { return m_IndexBuffer; }
	inline const std::vector<Ref<VertexBuffer>>& GetVertexBuffers()	const { return m_VertexBuffers; }

private:

	// --- Private Methods ---
	void SetFloatAttribute(const BufferElement& element, uint index, uint stride);
	void SetIntAttribute(const BufferElement& element, uint index, uint stride);
	void SetMatrixAttribute(const BufferElement& element, uint index, uint stride);

private:

	// --- Variables ---
	Ref<IndexBuffer> m_IndexBuffer = nullptr;
	std::vector<Ref<VertexBuffer>> m_VertexBuffers;

	uint m_ID = 0, m_VBufferIndex = 0;
};



// ---- Uniform Buffer ----
class UniformBuffer
{
public:

	// --- Des/Construction ---
	UniformBuffer(uint size, uint binding);
	~UniformBuffer();

	// --- Class Methods ---
	void Bind() const;
	void Unbind() const;

	// --- Getters/Setters ---
	void SetLayout(const BufferLayout& layout)			{ m_Layout = layout; }
	const BufferLayout& GetLayout()				const	{ return m_Layout; }
	
	void SetData(const std::string& element_name, const void* data);

private:

	// --- Variables ---
	uint m_ID = 0, m_Binding = 0;
	BufferLayout m_Layout;
};


#endif //_BUFFERS_H_