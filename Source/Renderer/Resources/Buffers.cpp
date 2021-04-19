#include "Buffers.h"

#include <glad/glad.h>


// ------------------------------------------------------------------------------
VertexBuffer::VertexBuffer(float* vertices, uint size)
{
	glCreateBuffers(1, &m_ID);
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW); // TODO: If I make a batch renderer, change this to dynamic
	//glBindBuffer(GL_ARRAY_BUFFER, 0); //TODO: Take a look at this unbind stuff! (all over the file!)
}

VertexBuffer::VertexBuffer(uint size)
{
	glCreateBuffers(1, &m_ID);
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
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
	glCreateBuffers(1, &m_ID);
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint), vertices, GL_STATIC_DRAW); // TODO: If I make a batch renderer, change this to dynamic
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	// GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
	// Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state. 
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
	glCreateVertexArrays(1, &m_ID);
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



// ------------------------------------------------------------------------------
UniformBuffer::UniformBuffer(BufferLayout layout, uint binding) : m_Layout(layout), m_Size(layout.GetStride())
{
	glCreateBuffers(1, &m_ID);
	glBindBuffer(GL_UNIFORM_BUFFER, m_ID);
	glBufferData(GL_UNIFORM_BUFFER, m_Size, NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_ID);
}

UniformBuffer::~UniformBuffer()
{
	glDeleteBuffers(1, &m_ID);
}

void UniformBuffer::Bind() const
{
	glBindBufferBase(GL_UNIFORM_BUFFER, m_Binding, m_ID);
}

void UniformBuffer::Unbind() const
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UniformBuffer::SetData(const std::string& element_name, const void* data) const
{
	for (const BufferElement& element : m_Layout.GetElements())
	{
		if (element.Name == element_name)
		{
			glBufferSubData(GL_UNIFORM_BUFFER, element.Offset, element.Size, data);
			break;
		}
	}
}



// ------------------------------------------------------------------------------
ShaderStorageBuffer::ShaderStorageBuffer(BufferLayout layout, uint binding) : m_Layout(layout), m_Size(layout.GetStride())
{
	glCreateBuffers(1, &m_ID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_Size, NULL, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_ID);
}

ShaderStorageBuffer::~ShaderStorageBuffer()
{
	glDeleteBuffers(1, &m_ID);
}

void ShaderStorageBuffer::Bind() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ID);
}

void ShaderStorageBuffer::Unbind() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ShaderStorageBuffer::SetData(const std::string& element_name, const void* data) const
{
	for (const BufferElement& element : m_Layout.GetElements())
	{
		if (element.Name == element_name)
		{
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, element.Offset, element.Size, data);
			break;
		}
	}
}