#include "core/mpr/mprState.h"
#include <vtkImageData.h>
#include <vtkResliceCursor.h>
#include <qDebug>

/*
    数据中心 持有唯一的vtkImagedata 和 vtkreslicecursor(是2d平面那两条互相垂直的线)
*/

//m_cursor: vtkResliceCursor 负责记录三向切片的光标位置与方向等信息
//m_image: vtkImageData 保存当前绑定的体数据
namespace core::mpr {
    MprState::MprState()
    {
		// vtkResliceCursor 负责记录切片的光标位置与方向等信息
        m_cursor = vtkResliceCursor::New();
    }

    MprState::~MprState()
    {
        if (m_cursor) {
            m_cursor->Delete();
            m_cursor = nullptr;
        }
    }

    void MprState::bindImage(vtkImageData* img)
    {
        m_image = img;
        if (m_cursor) {
            m_cursor->SetImage(img);
        }
    }

	//这个函数的作用是将 vtkResliceCursor 的中心 绑定到 vtkImageData 体数据的中心 
    void MprState::resetToCenter()
    {
        if (!m_cursor || !m_image) {
            return;
        }

        // 根据体数据的中心点重置reslice光标
        double bounds[6];
        m_image->GetBounds(bounds);
        const double center[3] = {
            0.5 * (bounds[0] + bounds[1]),
            0.5 * (bounds[2] + bounds[3]),
            0.5 * (bounds[4] + bounds[5]),
        };
        m_cursor->SetCenter(center[0], center[1], center[2]);
    }

    /*
        将世界坐标 world 转换成图像索引 index
        world = origin + index * specing
        ResliceCursor 给出的中心点是世界坐标 mm，
        但图像切片编号需要索引 index
        知道光标在三维中的位置 还要知道它对应的是第几张切片
    */

     int MprState::axialIndex() const
    {
        if (!m_cursor || !m_image) {
            return 0;
        }

        double c[3]; //光标在三维空间的实际位置
        m_cursor->GetCenter(c);

		double origin[3]; //第0号体素的物理位置
        m_image->GetOrigin(origin);//GetOrigin:获取图像数据的原点坐标 

		double spacing[3]; //
        m_image->GetSpacing(spacing);//GetSpacing:获取图像数据的体素间距
        return static_cast<int>(std::round((c[2] - origin[2]) / spacing[2]));
     }

    int MprState::coronalIndex() const
    {
        if (!m_cursor || !m_image) return 0;
        double c[3]; m_cursor->GetCenter(c);
        double origin[3]; m_image->GetOrigin(origin);
        double spacing[3]; m_image->GetSpacing(spacing);
        return static_cast<int>(std::round((c[1] - origin[1]) / spacing[1]));
    }

    int MprState::sagittalIndex() const
    {
        if (!m_cursor || !m_image) return 0;
        double c[3]; m_cursor->GetCenter(c);
        double origin[3]; m_image->GetOrigin(origin);
        double spacing[3]; m_image->GetSpacing(spacing);
        return static_cast<int>(std::round((c[0] - origin[0]) / spacing[0]));
    }

	void MprState::setIndices(int axial, int coronal, int sagittal)//传入索引
    {
        if (!m_cursor || !m_image) {
            return;
        }

        double origin[3];  m_image->GetOrigin(origin);
        double spacing[3]; m_image->GetSpacing(spacing);
        double c[3];       m_cursor->GetCenter(c);
		/*qDebug() << origin[0] << origin[1] << origin[2];*/

        c[2] = origin[2] + axial * spacing[2]; // axial->Z
        c[1] = origin[1] + coronal * spacing[1]; // coronal->Y
        c[0] = origin[0] + sagittal * spacing[0]; // sagittal->X

        m_cursor->SetCenter(c[0], c[1], c[2]);//知道了要去第几层
    }

    void MprState::center(double out[3]) const
    {
        if (!m_cursor || !out) {
            return;
        }

        m_cursor->GetCenter(out);
    }

    void MprState::setCenter(const double c[3])//实际距离 mm
    {
        if (!m_cursor || !c) {
            return;
        }

        m_cursor->SetCenter(c[0], c[1], c[2]);//算好了要去的毫米坐标
    }

    vtkResliceCursor* MprState::cursor() const
    {
        return m_cursor;
    }

    vtkImageData* MprState::image() const
    {
        return m_image;
    }

} // namespace core::mpr