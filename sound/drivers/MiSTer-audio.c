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
#include <sound/control.h>

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

static const struct regmap_config audio_config = {
	.name = "MiSTer-audio.data",
	.reg_bits = 32,
	.reg_stride = 4,
	.val_bits = 32
};

static int getreg(struct snd_kcontrol *kcontrol, int reg)
{
	int val = 0;
	int ret = regmap_read((struct regmap *)snd_kcontrol_chip(kcontrol), reg, &val);
	return (ret) ? -1 : val&0xf;
}

static int setreg(struct snd_kcontrol *kcontrol, int reg, int val)
{
	int ret = regmap_write((struct regmap *)snd_kcontrol_chip(kcontrol), reg, val);
	if(ret) return -1;
	return 0;
}

static int vol_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
        uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
        uinfo->count = 1;
        uinfo->value.integer.min = 0;
        uinfo->value.integer.max = 15;
        return 0;
}

static int vol_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int vol = getreg(kcontrol, 0);
	if(vol<0) return vol;

        ucontrol->value.integer.value[0] = vol;
        return 0;
}

static int vol_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
        int changed = 0;

	int vol = getreg(kcontrol, 0);
	if(vol<0) return vol;

	if(vol != ucontrol->value.integer.value[0]) 
	{
		vol = setreg(kcontrol, 0, ucontrol->value.integer.value[0]);
		if(vol<0) return vol;
		changed = 1;
	}
        return changed;
}

static int sw_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
        uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
        uinfo->count = 1;
        uinfo->value.integer.min = 0;
        uinfo->value.integer.max = 1;
        return 0;
}

static int sw_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	int sw = getreg(kcontrol, 4);
	if(sw<0) return sw;

        ucontrol->value.integer.value[0] = sw&1;
        return 0;
}

static int sw_put(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
        int changed = 0;

	int sw = getreg(kcontrol, 4);
	if(sw<0) return sw;

	if(sw != ucontrol->value.integer.value[0]) 
	{
		sw = setreg(kcontrol, 4, ucontrol->value.integer.value[0]);
		if(sw<0) return sw;
		changed = 1;
	}
        return changed;
}

static struct snd_kcontrol_new vol_control = {
        .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
        .name = "Master Playback Volume",
        .index = 0,
        .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
        .private_value = 0xffff,
        .info = vol_info,
        .get = vol_get,
        .put = vol_put
};

static struct snd_kcontrol_new sw_control = {
        .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
        .name = "Master Playback Switch",
        .index = 0,
        .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
        .private_value = 0xffff,
        .info = sw_info,
        .get = sw_get,
        .put = sw_put
};

static int audio_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *cpu_np;
	struct snd_soc_card *card = &snd_soc;
	void __iomem *base;
	struct regmap *regmap_data;
	struct resource *res;
	int ret;

	if (!np) return -ENODEV;

	card->dev = &pdev->dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "No memory resource\n");
		return -ENODEV;
	}
	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base)) {
		dev_err(&pdev->dev, "No ioremap resource\n");
		return PTR_ERR(base);
	}
	printk(KERN_ALERT "MiSTer-audio at %08x\n", (int)base);

	regmap_data = devm_regmap_init_mmio(&pdev->dev, base, &audio_config);
	if (IS_ERR(regmap_data)) {
		dev_err(&pdev->dev, "No regmap_data\n");
		return PTR_ERR(regmap_data);
	}

	platform_set_drvdata(pdev, regmap_data);

	cpu_np = of_parse_phandle(np, "cpu-link", 0);
	if (!cpu_np)
	{
		dev_err(&pdev->dev, "cpu-link info is missing\n");
		return -EINVAL;
	}
	dai.cpu_of_node = cpu_np;
	dai.platform_of_node = cpu_np;

	ret = snd_soc_register_card(card);
	if (ret)
	{
		dev_err(&pdev->dev, "snd_soc_register_card() failed\n");
		return ret;
	}

	ret = snd_ctl_add(card->snd_card, snd_ctl_new1(&vol_control, regmap_data));
	if (ret) dev_err(&pdev->dev, "snd_ctl_add(vol) failed(%d)\n", ret);

	ret = snd_ctl_add(card->snd_card, snd_ctl_new1(&sw_control, regmap_data));
	if (ret) dev_err(&pdev->dev, "snd_ctl_add(sw) failed(%d)\n", ret);

	dev_info(&pdev->dev, "audio_probe() OK\n");
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
	{ .compatible = "MiSTer,Audio", },
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
