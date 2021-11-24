idf_component_register(SRCS "main/i2c.cpp"
                            "main/GeneralUtils.cpp"
                       INCLUDE_DIRS "main/include"
                       REQUIRES nvs_flash)