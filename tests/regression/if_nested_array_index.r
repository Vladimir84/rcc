rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

foo <- c(2,3,4,5,6,7,8,9,1)
bar <- c(51,52,53,54,55,56,57,58,59,60)
if ((bar[foo[4]] - 1) == 54) 1984 else 2001

