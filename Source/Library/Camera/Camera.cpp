#include "Camera/Camera.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::Camera

      Summary:  Constructor

      Modifies: [m_yaw, m_pitch, m_moveLeftRight, m_moveBackForward,
                 m_moveUpDown, m_travelSpeed, m_rotationSpeed,
                 m_padding, m_cameraForward, m_cameraRight, m_cameraUp,
                 m_eye, m_at, m_up, m_rotation, m_view].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Camera::Camera definition (remove the comment)
    --------------------------------------------------------------------*/
    Camera::Camera(_In_ const XMVECTOR& position)
        : m_cameraForward(DEFAULT_FORWARD)
        , m_cameraRight(DEFAULT_RIGHT)
        , m_cameraUp(DEFAULT_UP)
        , m_moveLeftRight(0.0f)
        , m_moveBackForward(0.0f)
        , m_yaw(0.0f)
        , m_pitch(0.0f)
        , m_moveUpDown(0.0f)
        , m_travelSpeed(10.0f)
        , m_rotationSpeed(15.0f)
        , m_rotation()
        , m_view()
        , m_at()
        , m_up()
        , m_eye(position)
    {

    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::GetEye

      Summary:  Returns the eye vector

      Returns:  const XMVECTOR&
                  The eye vector
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Camera::GetEye definition (remove the comment)
    --------------------------------------------------------------------*/
    const XMVECTOR& Camera::GetEye() const
    {
        return m_eye;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::GetAt

      Summary:  Returns the at vector

      Returns:  const XMVECTOR&
                  The at vector
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Camera::GetAt definition (remove the comment)
    --------------------------------------------------------------------*/
    const XMVECTOR& Camera::GetAt() const
    {
        return m_at;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::GetUp

      Summary:  Returns the up vector

      Returns:  const XMVECTOR&
                  The up vector
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Camera::GetUp definition (remove the comment)
    --------------------------------------------------------------------*/
    const XMVECTOR& Camera::GetUp() const
    {
        return m_up;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::GetView

      Summary:  Returns the view matrix

      Returns:  const XMMATRIX&
                  The view matrix
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Camera::GetView definition (remove the comment)
    --------------------------------------------------------------------*/
    const XMMATRIX& Camera::GetView() const
    {
        return m_view;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::HandleInput

      Summary:  Sets the camera state according to the given input

      Args:     const DirectionsInput& directions
                  Keyboard directional input
                const MouseRelativeMovement& mouseRelativeMovement
                  Mouse relative movement input
                FLOAT deltaTime
                  Time difference of a frame

      Modifies: [m_yaw, m_pitch, m_moveLeftRight, m_moveBackForward,
                 m_moveUpDown].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Camera::HandleInput definition (remove the comment)
    --------------------------------------------------------------------*/
    void Camera::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        x = (FLOAT)(directions.bFront - directions.bBack);
        y = (FLOAT)(directions.bRight - directions.bLeft);
        z = (FLOAT)(directions.bUp - directions.bDown);

        if ((x != 0) || (y != 0) || (z != 0))
        {
            XMVECTOR DIR_Matrix = XMVectorSet(x, y, z, 0.0f);
            DIR_Matrix = XMVector3Normalize(DIR_Matrix);
            DIR_Matrix *= m_travelSpeed * deltaTime;
            XMFLOAT3 dir_move;
            XMStoreFloat3(&dir_move, DIR_Matrix);
            m_moveBackForward += dir_move.x;
            m_moveLeftRight += dir_move.y;
            m_moveUpDown += dir_move.z;
        }

        float rot_x = 0.0f;
        float rot_y = 0.0f;

        rot_x = mouseRelativeMovement.X;
        rot_y = mouseRelativeMovement.Y;
        if ((rot_x != 0) || (rot_y != 0))
        {
            XMVECTOR ROT_Matrix = XMVectorSet(rot_x, rot_y, 0.0f, 0.0f);
            ROT_Matrix = XMVector3Normalize(ROT_Matrix);
            ROT_Matrix *= m_rotationSpeed * deltaTime;
            XMFLOAT2 rot_move;
            XMStoreFloat2(&rot_move, ROT_Matrix);

            m_yaw += rot_move.x;
            m_pitch += rot_move.y;

            if (m_pitch > XM_PIDIV2)
            {
                m_pitch = XM_PIDIV2;
            }
            else if (m_pitch < -XM_PIDIV2)
            {
                m_pitch = -XM_PIDIV2;
            }
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::Update

      Summary:  Updates the camera based on its state

      Args:     FLOAT deltaTime
                  Time difference of a frame

      Modifies: [m_rotation, m_at, m_cameraRight, m_cameraUp,
                 m_cameraForward, m_eye, m_moveLeftRight,
                 m_moveBackForward, m_moveUpDown, m_up, m_view].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Camera::Update definition (remove the comment)
    --------------------------------------------------------------------*/
    void Camera::Update(_In_ FLOAT deltaTime)
    {
        m_rotation = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);
        m_at = XMVector3TransformCoord(DEFAULT_FORWARD, m_rotation);
        m_at = XMVector3Normalize(m_at);

        XMMATRIX tempMatrix;
        tempMatrix = XMMatrixRotationY(m_yaw);

        m_cameraRight = XMVector3TransformCoord(DEFAULT_RIGHT, tempMatrix);
        m_cameraUp = XMVector3TransformCoord(DEFAULT_UP, tempMatrix);
        m_cameraForward = XMVector3TransformCoord(DEFAULT_FORWARD, tempMatrix);

        m_eye += m_moveLeftRight * m_cameraRight;
        m_eye += m_moveBackForward * m_cameraForward;
        m_eye += m_moveUpDown * m_cameraUp;
        
        m_moveLeftRight = 0.0f;
        m_moveBackForward = 0.0f;
        m_moveUpDown = 0.0f;

        m_up = XMVector3TransformCoord(DEFAULT_UP, m_rotation);
        m_at = m_eye + m_at;
        m_view = XMMatrixLookAtLH(m_eye, m_at, m_up);
    }
}