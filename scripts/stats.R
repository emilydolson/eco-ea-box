library(readr)
library(dplyr)
library(ggplot2)
#library(logistf)
library(brglm)

all_data <- read_csv("~/repos/eco-ea-box/scripts/all_data.csv")
all_data$selection <- as.factor(all_data$selection)

results <- all_data %>% group_by(good, bad, selection) %>% summarize(suc = mean(success))
ggplot(data=results) + geom_raster(aes(x=good, y=bad, fill=suc))  + scale_x_continuous("Number of good hints", breaks=c(0,2,4,6,8,10))+ scale_y_continuous("Number of bad hints", breaks=c(0,2,4,6,8,10)) + facet_grid(~selection) + theme_classic() + scale_fill_gradientn(name="Proportion\nsuccessful   ", colors=c("#17164b", "#152767", "#0f3e7f", "#075a90", "#027898", "#049798", "#10b592", "#29cd8c", "#50e08a", "#81ee96")) + theme(legend.position = "bottom")
ggsave(filename = "eco-ea-heatmaps.png", width = 5.5, height = 3.3, units = "in")

all_data <- within(all_data, selection <- relevel(selection, ref = "Tournament"))
summary(all_data$selection)
model <- brglm(success ~ good+bad+selection+good:selection+bad:selection,family=binomial(link='logit'),data=all_data)
#model <- logistf(success ~ good+bad+selection+good:selection+bad:selection,data=all_data)
summary(model)
