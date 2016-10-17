library(ggplot2)
library(plyr)

args <- commandArgs(trailingOnly=TRUE)

df <- read.table(args[1], header=T,sep=",")

df32 <- df[df$type == "rdrand32",]

pl <- ggplot(df, aes(x=iteration, y=value)) +  
    geom_point(size=0.01) + 
    theme_bw() +
    theme(legend.title=element_blank(), legend.position="top") +
    facet_grid(type ~ ., scales="free")
ggsave("rdrand_dist.pdf", width=6, height=8, limitsize=FALSE)
