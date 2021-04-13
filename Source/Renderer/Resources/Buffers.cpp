#include "Buffers.h"

#include <glad/glad.h>


// ------------------------------------------------------------------------------
VertexBuffer::VertexBuffer(float* vertices, uint size)
{
	glGenBuffers(1, &m_ID);
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_DYNAMIC_DRAW); // TODO: If I make a batch renderer, change this to static
	//glBindBuffer(GL_ARRAY_BUFFER, 0); //TODO: Take a look at this unbind stuff! (all over the file!)
}

VertexBuffer::VertexBuffer(uint size)
{
	glGenBuffers(1, &m_ID);
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_ID);
}

void VertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
}

void VertexBuffer::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::SetData(const void* data, uint size)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
}



// ------------------------------------------------------------------------------
IndexBuffer::IndexBuffer(uint* vertices, uint count) : m_Count(count)
{
	glGenBuffers(1, &m_ID);
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint), vertices, GL_DYNAMIC_DRAW); // TODO: If I make a batch renderer, change this to static
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &m_ID);
}

void IndexBuffer::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
}

void IndexBuffer::Unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



// ------------------------------------------------------------------------------
VertexArray::VertexArray()
{
	glGenVertexArrays(1, &m_ID);
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &m_ID);
}

void VertexArray::Bind() const
{
	glBindVertexArray(m_ID);
}

void VertexArray::Unbind() const
{
	glBindVertexArray(0);
}


void VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& index_buffer)
{
	glBindVertexArray(m_ID);
	index_buffer->Bind();
	m_IndexBuffer = index_buffer;
	//glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertex_buffer)
{
	ASSERT(vertex_buffer->GetLayout().GetElements().size(), "Vertex Buffer has not layouts!");
	glBindVertexArray(m_ID);
	vertex_buffer->Bind();

	const auto& layout = vertex_buffer->GetLayout();
	for (const auto& element : layout)
	{
		switch (element.Type)
		{
			case SHADER_DATA::FLOAT:
			case SHADER_DATA::FLOAT2:
			case SHADER_DATA::FLOAT3:
			case SHADER_DATA::FLOAT4:
			{
				SetFloatAttribute(element, m_VBufferIndex, layout.GetStride());
				++m_VBufferIndex;
				break;
			}
			case SHADER_DATA::INT:
			case SHADER_DATA::INT2:
			case SHADER_DATA::INT3:
			case SHADER_DATA::INT4:
			case SHADER_DATA::BOOL:
			{
				SetIntAttribute(element, m_VBufferIndex, layout.GetStride());
				++m_VBufferIndex;
				break;
			}
			case SHADER_DATA::MAT3:
			case SHADER_DATA::MAT4:
			{
				SetMatrixAttribute(element, m_VBufferIndex, layout.GetStride());
				m_VBufferIndex += element.GetElementTypeCount();
				break;
			}
		}
	}

	m_VertexBuffers.push_back(vertex_buffer);
}


void VertexArray::SetFloatAttribute(const BufferElement& element, uint index, uint stride)
{
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, element.GetElementTypeCount(), ShaderDataTypeToOpenGLType(element.Type),
		element.Normalized ? GL_TRUE : GL_FALSE, stride, (const void*)element.Offset);
}

void VertexArray::SetIntAttribute(const BufferElement& element, uint index, uint stride)
{
	glEnableVertexAttribArray(index);
	glVertexAttribIPointer(index, element.GetElementTypeCount(), ShaderDataTypeToOpenGLType(element.Type), stride, (const void*)element.Offset);
}

void VertexArray::SetMatrixAttribute(const BufferElement& element, uint index, uint stride)
{
	uint8_t count = element.GetElementTypeCount();
	for (uint8_t i = 0; i < count; ++i, ++index)
	{
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, count, ShaderDataTypeToOpenGLType(element.Type),
			element.Normalized ? GL_TRUE : GL_FALSE, stride, (const void*)(element.Offset + sizeof(float) * count * i));

		glVertexAttribDivisor(index, 1);
	}
}