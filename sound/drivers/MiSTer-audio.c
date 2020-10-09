/*
 * Soundcard for MiSTer. (C) 2018 Sorgelig
 *
 * Licensed under the GPL-2.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

static int init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct device *dev = rtd->card->dev;
	unsigned int fmt;
	int ret;

	dev_dbg(dev, "init\n");

	fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS;

	ret = snd_soc_dai_set_fmt(cpu_dai, fmt);
	if (ret < 0) return ret;
	return 0;
}

static struct snd_soc_dai_link dai = {
	.name = "MiSTer",
	.stream_name = "MiSTer PCM",
	.codec_name = "snd-soc-dummy",
	.codec_dai_name = "snd-soc-dummy-dai",
	.codec_of_node = NULL,
	.init = init
};

static struct snd_soc_card snd_soc = {
	.name = "MiSTer-audio",
	.owner = THIS_MODULE,
	.dai_link = &dai,
	.num_links = 1
};

static int audio_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *cpu_np;
	struct snd_soc_card *card = &snd_soc;
	int ret;

	if (!np) return -ENODEV;

	card->dev = &pdev->dev;

	cpu_np = of_parse_phandle(np, "cpu-link", 0);
	if (!cpu_np)
	{
		dev_err(&pdev->dev, "cpu-link info is missing\n");
		return -EINVAL;
	}
	dai.cpu_of_node = cpu_np;
	dai.platform_of_node = cpu_np;

	ret = snd_soc_register_card(card);
	if (ret) dev_err(&pdev->dev, "snd_soc_register_card() failed\n");

	return ret;
}

static int audio_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	if(card) snd_soc_unregister_card(card);

	if(dai.cpu_of_node) of_node_put(dai.cpu_of_node);
	dai.cpu_of_node = 0;

	return 0;
}

static const struct of_device_id dt_ids[] = {
	{ .compatible = "MiSTer-audio", },
	{ }
};
MODULE_DEVICE_TABLE(of, dt_ids);

static struct platform_driver audio_driver = {
	.driver = {
		.name	= "MiSTer-audio",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(dt_ids),
	},
	.probe	= audio_probe,
	.remove	= audio_remove,
};

module_platform_driver(audio_driver);

/* Module information */
MODULE_AUTHOR("Sorgelig@MiSTer");
MODULE_DESCRIPTION("ALSA MiSTer-audio");
MODULE_LICENSE("GPL");
