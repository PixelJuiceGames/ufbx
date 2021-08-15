
UFBXT_FILE_TEST(maya_leading_comma)
#if UFBXT_IMPL
{
	ufbxt_assert(!strcmp(scene->metadata.creator.data, "FBX SDK/FBX Plugins version 2019.2"));
}
#endif

UFBXT_FILE_TEST(maya_zero_end)
#if UFBXT_IMPL
{
	ufbxt_assert(!strcmp(scene->metadata.creator.data, "FBX SDK/FBX Plugins version 2019.2"));
}
#endif

UFBXT_TEST(error_format_long)
#if UFBXT_IMPL
{
	char data[] = "Bad FBX";
	ufbx_error error;
	ufbx_scene *scene = ufbx_load_memory(data, sizeof(data), NULL, &error);
	ufbxt_assert(!scene);

	char error_buf[512];
	size_t length = ufbx_format_error(error_buf, sizeof(error_buf), &error);
	ufbxt_assert(strlen(error_buf) == length);

	size_t num_lines = 0;
	for (size_t i = 0; i < length; i++) {
		if (error_buf[i] == '\n') num_lines++;
	}
	ufbxt_assert(num_lines == error.stack_size + 1);
}
#endif

UFBXT_TEST(error_format_short)
#if UFBXT_IMPL
{
	char data[] = "Bad FBX";
	ufbx_error error;
	ufbx_scene *scene = ufbx_load_memory(data, sizeof(data), NULL, &error);
	ufbxt_assert(!scene);

	char error_buf[512];
	for (size_t buf_len = 0; buf_len <= ufbxt_arraycount(error_buf); buf_len++) {
		size_t ret_len = ufbx_format_error(error_buf, buf_len, &error);
		if (buf_len == 0) {
			ufbxt_assert(ret_len == 0);
			continue;
		}

		size_t str_len = strlen(error_buf);
		ufbxt_hintf("buf_len = %zu, ret_len = %zu, str_len = %zu", buf_len, ret_len, str_len);
		ufbxt_assert(ret_len == str_len);
		if (buf_len < 16) {
			ufbxt_assert(ret_len == buf_len - 1);
		}
	}
}
#endif

UFBXT_FILE_TEST(maya_node_attribute_zoo)
#if UFBXT_IMPL
{
	ufbx_node *node;

	node = ufbx_find_node(scene, "Null");
	ufbxt_assert(node && node->attrib_type == UFBX_ELEMENT_EMPTY);
	ufbxt_assert(node->attrib && node->attrib->type == UFBX_ELEMENT_EMPTY);

	node = ufbx_find_node(scene, "Mesh");
	ufbxt_assert(node && node->attrib_type == UFBX_ELEMENT_MESH);
	ufbxt_assert(node->attrib && node->attrib->type == UFBX_ELEMENT_MESH);
	ufbxt_assert(&node->mesh->element == node->attrib);

	node = ufbx_find_node(scene, "Bone");
	ufbxt_assert(node && node->attrib_type == UFBX_ELEMENT_BONE);
	ufbxt_assert(node->attrib && node->attrib->type == UFBX_ELEMENT_BONE);
	ufbxt_assert(&node->bone->element == node->attrib);
	ufbxt_assert_close_real(err, node->bone->length, 16.666667f); // TODO: /100?

	node = ufbx_find_node(scene, "Camera");
	ufbxt_assert(node && node->attrib_type == UFBX_ELEMENT_CAMERA);
	ufbxt_assert(node->attrib && node->attrib->type == UFBX_ELEMENT_CAMERA);
	ufbxt_assert(&node->camera->element == node->attrib);
	ufbxt_assert(node->camera->gate_fit == UFBX_GATE_FIT_FILL);
	ufbxt_assert_close_real(err, node->camera->field_of_view_deg.x / 10.0f, 54.43f / 10.0f);
	ufbxt_assert_close_real(err, node->camera->focal_length_mm, 35.0f);

	node = ufbx_find_node(scene, "Light");
	ufbxt_assert(node && node->attrib_type == UFBX_ELEMENT_LIGHT);
	ufbxt_assert(node->attrib && node->attrib->type == UFBX_ELEMENT_LIGHT);
	ufbxt_assert(&node->light->element == node->attrib);
	ufbxt_assert_close_real(err, node->light->intensity, 1.0f);
	ufbxt_assert_close_real(err, node->light->color.x, 1.0f);
	ufbxt_assert_close_real(err, node->light->color.y, 1.0f);
	ufbxt_assert_close_real(err, node->light->color.z, 1.0f);

	node = ufbx_find_node(scene, "StereoCamera");
	ufbxt_assert(node && node->attrib_type == UFBX_ELEMENT_CAMERA_STEREO);
	ufbxt_assert(node->attrib && node->attrib->type == UFBX_ELEMENT_CAMERA_STEREO);
	ufbx_camera_stereo *stereo = (ufbx_camera_stereo*)node->attrib;
	ufbxt_assert(stereo->left && stereo->left->element.type == UFBX_ELEMENT_CAMERA);
	ufbxt_assert(stereo->right && stereo->right->element.type == UFBX_ELEMENT_CAMERA);
	ufbx_prop left_focal_prop = ufbx_evaluate_prop(scene->anim, &stereo->left->element, "FocalLength", 0.5);
	ufbx_prop right_focal_prop = ufbx_evaluate_prop(scene->anim, &stereo->right->element, "FocalLength", 0.5);
	ufbxt_assert_close_real(err, left_focal_prop.value_real, 42.011f);
	ufbxt_assert_close_real(err, right_focal_prop.value_real, 42.011f);

	node = ufbx_find_node(scene, "NurbsCurve");
	ufbxt_assert(node && node->attrib_type == UFBX_ELEMENT_NURBS_CURVE);
	ufbxt_assert(node->attrib && node->attrib->type == UFBX_ELEMENT_NURBS_CURVE);

	node = ufbx_find_node(scene, "NurbsSurface");
	ufbxt_assert(node && node->attrib_type == UFBX_ELEMENT_NURBS_SURFACE);
	ufbxt_assert(node->attrib && node->attrib->type == UFBX_ELEMENT_NURBS_SURFACE);

	node = ufbx_find_node(scene, "NurbsTrim");
	ufbxt_assert(node && node->attrib_type == UFBX_ELEMENT_NURBS_TRIM_SURFACE);
	ufbxt_assert(node->attrib && node->attrib->type == UFBX_ELEMENT_NURBS_TRIM_SURFACE);

	node = ufbx_find_node(scene, "LodGroup");
	ufbxt_assert(node && node->attrib_type == UFBX_ELEMENT_LOD_GROUP);
	ufbxt_assert(node->attrib && node->attrib->type == UFBX_ELEMENT_LOD_GROUP);
}
#endif

UFBXT_FILE_TEST(synthetic_duplicate_prop)
#if UFBXT_IMPL
{
	ufbx_prop *prop;
	ufbx_node *cube = ufbx_find_node(scene, "pCube1");
	ufbxt_assert(cube);

	prop = ufbx_find_prop(&cube->props, "Lcl Translation");
	ufbxt_assert(prop);
	ufbxt_assert_close_real(err, prop->value_vec3.x, -1.0f);

	prop = ufbx_find_prop(&cube->props, "Lcl Scaling");
	ufbxt_assert(prop);
	ufbxt_assert_close_real(err, prop->value_vec3.x, 2.0f);
}
#endif

UFBXT_FILE_TEST(synthetic_missing_version)
#if UFBXT_IMPL
{
	ufbxt_assert(scene->metadata.version == 6100);
	ufbxt_assert(scene->metadata.exporter == UFBX_EXPORTER_FBX_SDK);
	ufbxt_assert(scene->metadata.exporter_version == ufbx_pack_version(2007, 2, 28));
}
#endif

UFBXT_FILE_TEST(synthetic_missing_exporter)
#if UFBXT_IMPL
{
	ufbxt_assert(scene->metadata.version == 6100);
	ufbxt_assert(scene->metadata.exporter == UFBX_EXPORTER_UNKNOWN);
	ufbxt_assert(scene->metadata.exporter_version == 0);
}
#endif

UFBXT_FILE_TEST(synthetic_blender_old_exporter)
#if UFBXT_IMPL
{
	ufbxt_assert(scene->metadata.version == 6100);
	ufbxt_assert(scene->metadata.exporter == UFBX_EXPORTER_UNKNOWN);
	ufbxt_assert(scene->metadata.exporter_version == 0);
}
#endif

UFBXT_FILE_TEST(blender_272_cube)
#if UFBXT_IMPL
{
	ufbxt_assert(scene->metadata.version == 7400);
	ufbxt_assert(scene->metadata.exporter == UFBX_EXPORTER_BLENDER_BINARY);
	ufbxt_assert(scene->metadata.exporter_version == ufbx_pack_version(2, 72, 0));
}
#endif

UFBXT_TEST(unicode_filename)
#if UFBXT_IMPL
{
	char buf[1024];
	int len = snprintf(buf, sizeof(buf), "%ssynthetic_\x61\xce\xb2\xe3\x82\xab\xf0\x9f\x98\x82_7500_ascii.fbx", data_root);
	ufbxt_assert(len > 0 && len < sizeof(buf));

	{
		ufbx_scene *scene = ufbx_load_file(buf, NULL, NULL);
		ufbxt_assert(scene);
		ufbxt_assert(ufbx_find_node(scene, "pCube1"));
		ufbxt_check_scene(scene);
		ufbx_free_scene(scene);
	}

	// Remove terminating \0 for explicit length test
	buf[len] = 'x';

	{
		ufbx_scene *scene = ufbx_load_file_len(buf, (size_t)len, NULL, NULL);
		ufbxt_assert(scene);
		ufbxt_assert(ufbx_find_node(scene, "pCube1"));
		ufbxt_check_scene(scene);
		ufbx_free_scene(scene);
	}
}
#endif
