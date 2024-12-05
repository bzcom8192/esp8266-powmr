Import("env")

env.Replace(PROGNAME="fw_esp-powmr_v%s" % env.GetProjectOption("custom_prog_version"))