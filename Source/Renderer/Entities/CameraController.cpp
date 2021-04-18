#include "CameraController.h"
#include "Core/Platform/Input.h"


// ------------------------------------------------------------------------------
CameraController::CameraController()
{
	RecalculateView();
	m_Camera.CalculateProjectionMatrix();
}

CameraController::CameraController(Camera& camera)
{
	RecalculateView();
	m_Camera.CalculateProjectionMatrix();
}

void CameraController::OnMouseScroll(float scroll)
{
	if (!Input::IsMouseButtonPressed(MOUSE::RIGHT))
		ZoomCamera(scroll * 0.1f);

	float scroll_pow = scroll * m_SpeedMultiplier / 8.0f;
	m_SpeedMultiplier += scroll_pow;	
}

void CameraController::OnUpdate(float dt)
{
	// -- Focus --
	if (Input::IsKeyPressed(KEY::F))
	{
		SetPosition(glm::vec3(0.0f) - GetForwardVector() * m_ZoomLevel);
		SetOrientation(0.5f, 0.0f);
		return;
	}

	// -- Mouse Movement Calc --
	const glm::vec2& mouse_pos = Input::GetMousePos();
	glm::vec2 delta = (mouse_pos - m_InitialMousePosition) * 0.003f;


	// -- Mid Mouse Button --
	if (Input::IsMouseButtonPressed(MOUSE::MIDDLE))
		PanCamera(delta);


	// -- Left & Right Mouse Buttons --
	if (Input::IsMouseButtonPressed(MOUSE::LEFT) && Input::IsMouseButtonPressed(MOUSE::RIGHT))
	{
		if (Input::IsKeyPressed(KEY::ALT))
			ZoomCamera(delta.y);
		else
			PanCamera(-delta);
	}
	else if (Input::IsMouseButtonPressed(MOUSE::LEFT) && Input::IsKeyPressed(KEY::ALT))
		OrbitCamera(delta);
	else if (Input::IsMouseButtonPressed(MOUSE::RIGHT))
	{
		if (Input::IsKeyPressed(KEY::ALT))
			ZoomCamera(delta.y);
		else
		{
			RotateCamera(delta);

			// - FPS Movement -
			float speed = m_MoveSpeed * m_SpeedMultiplier * dt;
			if (Input::IsKeyPressed(KEY::SHIFT))
				speed *= 2.0f;

			if (Input::IsKeyPressed(KEY::W))
				SetPosition(m_Position + GetForwardVector() * speed);
			if (Input::IsKeyPressed(KEY::S))
				SetPosition(m_Position - GetForwardVector() * speed);
			if (Input::IsKeyPressed(KEY::A))
				SetPosition(m_Position - GetRightVector() * speed);
			if (Input::IsKeyPressed(KEY::D))
				SetPosition(m_Position + GetRightVector() * speed);
			if (Input::IsKeyPressed(KEY::Q))
				SetPosition(m_Position - GetUpVector() * speed);
			if (Input::IsKeyPressed(KEY::E))
				SetPosition(m_Position + GetUpVector() * speed);
		}
	}

	m_InitialMousePosition = mouse_pos;
}



// ------------------------------------------------------------------------------
void CameraController::RecalculateView()
{
	m_Position = m_FocalPoint - GetForwardVector() * m_ZoomLevel;
	m_Camera.CalculateViewMatrix(m_Position, GetOrientation());
}



// ------------------------------------------------------------------------------
void CameraController::SetOrientation(float x_angle, float y_angle)
{
	m_Pitch = x_angle;
	m_Yaw = y_angle;
	m_FocalPoint = m_Position + GetForwardVector() * m_ZoomLevel;
	RecalculateView();
}


void CameraController::SetPosition(const glm::vec3& position)
{
	m_Position = position;
	m_FocalPoint = m_Position + GetForwardVector() * m_ZoomLevel;
	RecalculateView();
}


void CameraController::CalculateRotation(const glm::vec2 rotation)
{
	float yaw = GetUpVector().y < 0.0f ? -1.0f : 1.0f;
	m_Yaw += yaw * rotation.x * m_RotationSpeed;
	m_Pitch += rotation.y * m_RotationSpeed;
	m_Pitch = glm::clamp(m_Pitch, -1.56f, 1.56f);
}


void CameraController::RotateCamera(const glm::vec2& rotation)
{
	CalculateRotation(rotation);
	m_FocalPoint = m_Position + GetForwardVector() * m_ZoomLevel;
	RecalculateView();
}


void CameraController::OrbitCamera(const glm::vec2& rotation)
{
	CalculateRotation(rotation);
	RecalculateView();
}


void CameraController::PanCamera(const glm::vec2& panning)
{
	// -- Pan Speed on X and Y --
	glm::ivec2 viewport = m_Camera.GetViewportSize();
	float x = std::min((float)viewport.x / 1000.0f, m_PanSpeed);
	float y = std::min((float)viewport.y / 1000.0f, m_PanSpeed);

	// This is a quadratic function: X squared - X * constant
	float dx = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;
	float dy = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

	// -- Pan --
	m_FocalPoint += -GetRightVector() * panning.x * dx * m_ZoomLevel;
	m_FocalPoint += GetUpVector() * panning.y * dy * m_ZoomLevel;
	RecalculateView();
}


void CameraController::ZoomCamera(float zoom)
{
	// -- Zoom Calc --
	float d = std::max(m_ZoomLevel * 0.35f, 0.0f);
	m_ZoomLevel -= zoom * std::min(d * d, 100.0f);
	m_ZoomLevel = glm::clamp(m_ZoomLevel, 1.75f, 75.0f);

	// -- View Recalc --
	RecalculateView();
}