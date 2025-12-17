#pragma once
#include <vector>
#include <array>

namespace core::data {
    class VolumeModel;   
}

namespace core::services {

    /*
     *  绑定当前的VolumeModel DICOM 到 vtkImageData的那套
     *  记录所有两点距离测量
     *  提供 IJK->世界坐标->mm 距离的计算
     */
    class DistanceMeasureService
    {
    public:
        struct Point3D
        {
            double x = 0.0;
            double y = 0.0;
            double z = 0.0;
        };

        // 一条距离测量记录
        struct DistanceItem
        {
            int id = 0;       
            Point3D p0World;  // 起点世界坐标
            Point3D p1World;  // 终点世界坐标
            double lengthMm;  // 两点之间的直线距离
        };

        DistanceMeasureService();

        /*
         * 绑定当前体数据
         */
        void bindVolume(const core::data::VolumeModel* volume);

        /*
         * 用体素索引（IJK）添加一条距离测量
         * 传入的是体素坐标，内部会根据 VolumeModel 的 spacing / origin / directionMatrix
         * 转成世界坐标，然后计算距离
         */
        int addDistanceByVoxel(const std::array<int, 3>& p0Ijk,
            const std::array<int, 3>& p1Ijk);

        /*
         * 直接用世界坐标添加一条距离测量
         */
        int addDistanceByWorld(const Point3D& p0World,
            const Point3D& p1World);

        /*
         * 获取所有测量记录
         */
        const std::vector<DistanceItem>& items() const;

        /*
         * 清空所有测量记录 析构
         */
        void clear();

    private:
        const core::data::VolumeModel* volume_ = nullptr; // 当前绑定的体数据
        std::vector<DistanceItem> items_;
        int nextId_ = 1;                                  // 下一个可用ID

        /*
         * 将体素坐标 (i, j, k) 转成世界坐标
         * 假设 VolumeModel::directionMatrix()返回的是按行展开的 3x3 矩阵
         */
        Point3D voxelToWorld(const std::array<int, 3>& ijk) const;

        /*
         * 计算两点之间的欧式距离
         */
        static double distance(const Point3D& a, const Point3D& b);
    };
} // namespace core::services
