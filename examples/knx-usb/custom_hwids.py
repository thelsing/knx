Import("env")

# access to global build environment
print(env)

board_config = env.BoardConfig()
board_config.update("build.hwids", [
#  ["0x135e", "0x0024"] # Merten GmbH & Co. KG
#  ["0x0E77", "0x2001"] # Weinzierl Engineering GmbH
  ["0x7660", "0x0002"] # KNX Assoc.
])
