// This file is part of the AliceVision project and is made available under
// the terms of the MPL2 license (see the COPYING.md file).

#include "aliceVision/sfm/sfm.hpp"
#include "testing/testing.h"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <sstream>

using namespace aliceVision;
using namespace aliceVision::cameras;
using namespace aliceVision::geometry;
using namespace aliceVision::sfm;

// Create a SfM scene with desired count of views & poses & intrinsic (shared or not)
// Add a 3D point with observation in 2 view (just in order to have non empty data)
SfM_Data create_test_scene(IndexT viewsCount, bool bSharedIntrinsic)
{
  SfM_Data sfm_data;
  sfm_data.s_root_path = "./";

  for(IndexT i = 0; i < viewsCount; ++i)
  {
    // Add views
    std::ostringstream os;
    os << "dataset/" << i << ".jpg";
    const IndexT id_view = i, id_pose = i;
    const IndexT id_intrinsic = bSharedIntrinsic ? 0 : i; //(shared or not intrinsics)

    std::shared_ptr<View> view = std::make_shared<View>(os.str(),id_view, id_intrinsic, id_pose, 1000, 1000);

    sfm_data.views[id_view] = view;

    // Add poses
    sfm_data.setPose(*view, Pose3());

    // Add intrinsics
    if (bSharedIntrinsic)
    {
      if (i == 0)
        sfm_data.intrinsics[0] = std::make_shared<Pinhole_Intrinsic>();
    }
    else
    {
      sfm_data.intrinsics[i] = std::make_shared<Pinhole_Intrinsic>();
    }
  }

  // Fill with not meaningful tracks
  Observations observations;
  observations[0] = Observation( Vec2(10,20), 0);
  observations[1] = Observation( Vec2(30,10), 1);
  sfm_data.structure[0].observations = observations;
  sfm_data.structure[0].X = Vec3(11,22,33);
  return sfm_data;
}

TEST(SfM_Data_IO, SAVE_LOAD_JSON) {

  const std::vector<std::string> ext_Type = {"json", "bin", "xml"};

  for (int i=0; i < ext_Type.size(); ++i)
  {
    std::ostringstream os;
    os << "SAVE_LOAD" << "." << ext_Type[i];
    const std::string filename = os.str();
    ALICEVISION_LOG_DEBUG("Testing:" << filename);

  // SAVE
  {
    const SfM_Data sfm_data = create_test_scene(2, true);
    EXPECT_TRUE( Save(sfm_data, filename, ALL) );
  }

  // LOAD
  {
    const SfM_Data sfm_data = create_test_scene(2, true);
    EXPECT_TRUE( Save(sfm_data, filename, ALL) );
    SfM_Data sfm_data_load;
    ESfM_Data flags_part = ALL;
    EXPECT_TRUE( Load(sfm_data_load, filename, flags_part) );
    EXPECT_EQ( sfm_data_load.views.size(), sfm_data.views.size());
    EXPECT_EQ( sfm_data_load.GetPoses().size(), sfm_data.GetPoses().size());
    EXPECT_EQ( sfm_data_load.intrinsics.size(), sfm_data.intrinsics.size());
    EXPECT_EQ( sfm_data_load.structure.size(), sfm_data.structure.size());
    EXPECT_EQ( sfm_data_load.control_points.size(), sfm_data.control_points.size());
  }

  // LOAD (only a subpart: VIEWS)
  {
    const SfM_Data sfm_data = create_test_scene(2, true);
    EXPECT_TRUE( Save(sfm_data, filename, ALL) );
    SfM_Data sfm_data_load;
    ESfM_Data flags_part = VIEWS;
    EXPECT_TRUE( Load(sfm_data_load, filename, flags_part) );
    EXPECT_EQ( sfm_data_load.views.size(), sfm_data.views.size());
    EXPECT_EQ( sfm_data_load.GetPoses().size(), 0);
    EXPECT_EQ( sfm_data_load.intrinsics.size(), 0);
    EXPECT_EQ( sfm_data_load.structure.size(), 0);
    EXPECT_EQ( sfm_data_load.control_points.size(), 0);
  }

  // LOAD (only a subpart: POSES)
  {
    const SfM_Data sfm_data = create_test_scene(2, true);
    EXPECT_TRUE( Save(sfm_data, filename, ALL) );
    SfM_Data sfm_data_load;
    ESfM_Data flags_part = EXTRINSICS;
    EXPECT_TRUE( Load(sfm_data_load, filename, flags_part) );
    EXPECT_EQ( sfm_data_load.views.size(), 0);
    EXPECT_EQ( sfm_data_load.GetPoses().size(), sfm_data.GetPoses().size());
    EXPECT_EQ( sfm_data_load.intrinsics.size(), 0);
    EXPECT_EQ( sfm_data_load.structure.size(), 0);
    EXPECT_EQ( sfm_data_load.control_points.size(), 0);
  }

  // LOAD (only a subpart: INTRINSICS)
  {
    const SfM_Data sfm_data = create_test_scene(2, true);
    EXPECT_TRUE( Save(sfm_data, filename, ALL) );
    SfM_Data sfm_data_load;
    ESfM_Data flags_part = INTRINSICS;
    EXPECT_TRUE( Load(sfm_data_load, filename, flags_part) );
    EXPECT_EQ( sfm_data_load.views.size(), 0);
    EXPECT_EQ( sfm_data_load.GetPoses().size(), 0);
    EXPECT_EQ( sfm_data_load.intrinsics.size(), sfm_data.intrinsics.size());
    EXPECT_EQ( sfm_data_load.structure.size(), 0);
    EXPECT_EQ( sfm_data_load.control_points.size(), 0);
  }

  // LOAD (subparts: COMBINED)
  {
    const SfM_Data sfm_data = create_test_scene(2,false); //2 intrinsics group here
    EXPECT_TRUE( Save(sfm_data, filename, ALL) );
    SfM_Data sfm_data_load;
    ESfM_Data flags_part = ESfM_Data(INTRINSICS | EXTRINSICS);
    EXPECT_TRUE( Load(sfm_data_load, filename, flags_part) );
    EXPECT_EQ( sfm_data_load.views.size(), 0);
    EXPECT_EQ( sfm_data_load.GetPoses().size(), sfm_data.GetPoses().size());
    EXPECT_EQ( sfm_data_load.intrinsics.size(), sfm_data.intrinsics.size());
    EXPECT_EQ( sfm_data_load.structure.size(), 0);
    EXPECT_EQ( sfm_data_load.control_points.size(), 0);
  }

  // LOAD (subparts: COMBINED)
  {
    const SfM_Data sfm_data = create_test_scene(2, true);
    EXPECT_TRUE( Save(sfm_data, filename, ALL) );
    SfM_Data sfm_data_load;
    ESfM_Data flags_part = ESfM_Data(VIEWS | INTRINSICS | EXTRINSICS);
    EXPECT_TRUE( Load(sfm_data_load, filename, flags_part) );
    EXPECT_EQ( sfm_data_load.views.size(), sfm_data.views.size());
    EXPECT_EQ( sfm_data_load.GetPoses().size(), sfm_data.GetPoses().size());
    EXPECT_EQ( sfm_data_load.intrinsics.size(), sfm_data.intrinsics.size());
    EXPECT_EQ( sfm_data_load.structure.size(), 0);
    EXPECT_EQ( sfm_data_load.control_points.size(), 0);
    }
  }
}

TEST(SfM_Data_IO, SAVE_PLY) {

  // SAVE as PLY
  {
    std::ostringstream os;
    os << "SAVE_LOAD" << ".ply";
    const std::string filename = os.str();
    ALICEVISION_LOG_DEBUG("Testing:" << filename);

    const SfM_Data sfm_data = create_test_scene(2, true);
    ESfM_Data flags_part = ESfM_Data(EXTRINSICS | STRUCTURE);
    EXPECT_TRUE( Save(sfm_data, filename, flags_part) );
    EXPECT_TRUE( stlplus::is_file(filename) );
  }
}

/* ************************************************************************* */
int main() { TestResult tr; return TestRegistry::runAllTests(tr);}
/* ************************************************************************* */
