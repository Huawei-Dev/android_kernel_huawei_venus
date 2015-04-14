/*
 * HD-audio core stuff
 */

#ifndef __SOUND_HDAUDIO_H
#define __SOUND_HDAUDIO_H

#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/timecounter.h>
#include <sound/core.h>
#include <sound/memalloc.h>
#include <sound/hda_verbs.h>

/* codec node id */
typedef u16 hda_nid_t;

struct hdac_bus;
struct hdac_stream;
struct hdac_device;
struct hdac_driver;
struct hdac_widget_tree;

/*
 * exported bus type
 */
extern struct bus_type snd_hda_bus_type;

/*
 * HDA device table
 */
struct hda_device_id {
	__u32 vendor_id;
	__u32 rev_id;
	const char *name;
	unsigned long driver_data;
};

/*
 * generic arrays
 */
struct snd_array {
	unsigned int used;
	unsigned int alloced;
	unsigned int elem_size;
	unsigned int alloc_align;
	void *list;
};

/*
 * HD-audio codec base device
 */
struct hdac_device {
	struct device dev;
	int type;
	struct hdac_bus *bus;
	unsigned int addr;		/* codec address */
	struct list_head list;		/* list point for bus codec_list */

	hda_nid_t afg;			/* AFG node id */
	hda_nid_t mfg;			/* MFG node id */

	/* ids */
	unsigned int vendor_id;
	unsigned int subsystem_id;
	unsigned int revision_id;
	unsigned int afg_function_id;
	unsigned int mfg_function_id;
	unsigned int afg_unsol:1;
	unsigned int mfg_unsol:1;

	unsigned int power_caps;	/* FG power caps */

	const char *vendor_name;	/* codec vendor name */
	const char *chip_name;		/* codec chip name */

	/* verb exec op override */
	int (*exec_verb)(struct hdac_device *dev, unsigned int cmd,
			 unsigned int flags, unsigned int *res);

	/* widgets */
	unsigned int num_nodes;
	hda_nid_t start_nid, end_nid;

	/* misc flags */
	atomic_t in_pm;		/* suspend/resume being performed */

	/* sysfs */
	struct hdac_widget_tree *widgets;

	/* regmap */
	struct regmap *regmap;
	struct snd_array vendor_verbs;
	bool lazy_cache:1;	/* don't wake up for writes */
	bool caps_overwriting:1; /* caps overwrite being in process */
	bool cache_coef:1;	/* cache COEF read/write too */
};

/* device/driver type used for matching */
enum {
	HDA_DEV_CORE,
	HDA_DEV_LEGACY,
};

/* direction */
enum {
	HDA_INPUT, HDA_OUTPUT
};

#define dev_to_hdac_dev(_dev)	container_of(_dev, struct hdac_device, dev)

int snd_hdac_device_init(struct hdac_device *dev, struct hdac_bus *bus,
			 const char *name, unsigned int addr);
void snd_hdac_device_exit(struct hdac_device *dev);
int snd_hdac_device_register(struct hdac_device *codec);
void snd_hdac_device_unregister(struct hdac_device *codec);

int snd_hdac_refresh_widgets(struct hdac_device *codec);

unsigned int snd_hdac_make_cmd(struct hdac_device *codec, hda_nid_t nid,
			       unsigned int verb, unsigned int parm);
int snd_hdac_exec_verb(struct hdac_device *codec, unsigned int cmd,
		       unsigned int flags, unsigned int *res);
int snd_hdac_read(struct hdac_device *codec, hda_nid_t nid,
		  unsigned int verb, unsigned int parm, unsigned int *res);
int _snd_hdac_read_parm(struct hdac_device *codec, hda_nid_t nid, int parm,
			unsigned int *res);
int snd_hdac_read_parm_uncached(struct hdac_device *codec, hda_nid_t nid,
				int parm);
int snd_hdac_override_parm(struct hdac_device *codec, hda_nid_t nid,
			   unsigned int parm, unsigned int val);
int snd_hdac_get_connections(struct hdac_device *codec, hda_nid_t nid,
			     hda_nid_t *conn_list, int max_conns);
int snd_hdac_get_sub_nodes(struct hdac_device *codec, hda_nid_t nid,
			   hda_nid_t *start_id);

/**
 * snd_hdac_read_parm - read a codec parameter
 * @codec: the codec object
 * @nid: NID to read a parameter
 * @parm: parameter to read
 *
 * Returns -1 for error.  If you need to distinguish the error more
 * strictly, use _snd_hdac_read_parm() directly.
 */
static inline int snd_hdac_read_parm(struct hdac_device *codec, hda_nid_t nid,
				     int parm)
{
	unsigned int val;

	return _snd_hdac_read_parm(codec, nid, parm, &val) < 0 ? -1 : val;
}

#ifdef CONFIG_PM
void snd_hdac_power_up(struct hdac_device *codec);
void snd_hdac_power_down(struct hdac_device *codec);
void snd_hdac_power_up_pm(struct hdac_device *codec);
void snd_hdac_power_down_pm(struct hdac_device *codec);
#else
static inline void snd_hdac_power_up(struct hdac_device *codec) {}
static inline void snd_hdac_power_down(struct hdac_device *codec) {}
static inline void snd_hdac_power_up_pm(struct hdac_device *codec) {}
static inline void snd_hdac_power_down_pm(struct hdac_device *codec) {}
#endif

/*
 * HD-audio codec base driver
 */
struct hdac_driver {
	struct device_driver driver;
	int type;
	const struct hda_device_id *id_table;
	int (*match)(struct hdac_device *dev, struct hdac_driver *drv);
	void (*unsol_event)(struct hdac_device *dev, unsigned int event);
};

#define drv_to_hdac_driver(_drv) container_of(_drv, struct hdac_driver, driver)

const struct hda_device_id *
hdac_get_device_id(struct hdac_device *hdev, struct hdac_driver *drv);

/*
 * Bus verb operators
 */
struct hdac_bus_ops {
	/* send a single command */
	int (*command)(struct hdac_bus *bus, unsigned int cmd);
	/* get a response from the last command */
	int (*get_response)(struct hdac_bus *bus, unsigned int addr,
			    unsigned int *res);
};

/*
 * Lowlevel I/O operators
 */
struct hdac_io_ops {
	/* mapped register accesses */
	void (*reg_writel)(u32 value, u32 __iomem *addr);
	u32 (*reg_readl)(u32 __iomem *addr);
	void (*reg_writew)(u16 value, u16 __iomem *addr);
	u16 (*reg_readw)(u16 __iomem *addr);
	void (*reg_writeb)(u8 value, u8 __iomem *addr);
	u8 (*reg_readb)(u8 __iomem *addr);
};

#define HDA_UNSOL_QUEUE_SIZE	64
#define HDA_MAX_CODECS		8	/* limit by controller side */

/* HD Audio class code */
#define PCI_CLASS_MULTIMEDIA_HD_AUDIO	0x0403

/*
 * CORB/RIRB
 *
 * Each CORB entry is 4byte, RIRB is 8byte
 */
struct hdac_rb {
	__le32 *buf;		/* virtual address of CORB/RIRB buffer */
	dma_addr_t addr;	/* physical address of CORB/RIRB buffer */
	unsigned short rp, wp;	/* RIRB read/write pointers */
	int cmds[HDA_MAX_CODECS];	/* number of pending requests */
	u32 res[HDA_MAX_CODECS];	/* last read value */
};

/*
 * HD-audio bus base driver
 */
struct hdac_bus {
	struct device *dev;
	const struct hdac_bus_ops *ops;
	const struct hdac_io_ops *io_ops;

	/* h/w resources */
	unsigned long addr;
	void __iomem *remap_addr;
	int irq;

	/* codec linked list */
	struct list_head codec_list;
	unsigned int num_codecs;

	/* link caddr -> codec */
	struct hdac_device *caddr_tbl[HDA_MAX_CODEC_ADDRESS + 1];

	/* unsolicited event queue */
	u32 unsol_queue[HDA_UNSOL_QUEUE_SIZE * 2]; /* ring buffer */
	unsigned int unsol_rp, unsol_wp;
	struct work_struct unsol_work;

	/* bit flags of detected codecs */
	unsigned long codec_mask;

	/* bit flags of powered codecs */
	unsigned long codec_powered;

	/* CORB/RIRB */
	struct hdac_rb corb;
	struct hdac_rb rirb;
	unsigned int last_cmd[HDA_MAX_CODECS];	/* last sent command */

	/* CORB/RIRB and position buffers */
	struct snd_dma_buffer rb;
	struct snd_dma_buffer posbuf;

	/* hdac_stream linked list */
	struct list_head stream_list;

	/* operation state */
	bool chip_init:1;		/* h/w initialized */

	/* behavior flags */
	bool sync_write:1;		/* sync after verb write */
	bool use_posbuf:1;		/* use position buffer */
	bool snoop:1;			/* enable snooping */
	bool align_bdle_4k:1;		/* BDLE align 4K boundary */
	bool reverse_assign:1;		/* assign devices in reverse order */
	bool corbrp_self_clear:1;	/* CORBRP clears itself after reset */

	int bdl_pos_adj;		/* BDL position adjustment */

	/* locks */
	spinlock_t reg_lock;
	struct mutex cmd_mutex;
};

int snd_hdac_bus_init(struct hdac_bus *bus, struct device *dev,
		      const struct hdac_bus_ops *ops,
		      const struct hdac_io_ops *io_ops);
void snd_hdac_bus_exit(struct hdac_bus *bus);
int snd_hdac_bus_exec_verb(struct hdac_bus *bus, unsigned int addr,
			   unsigned int cmd, unsigned int *res);
int snd_hdac_bus_exec_verb_unlocked(struct hdac_bus *bus, unsigned int addr,
				    unsigned int cmd, unsigned int *res);
void snd_hdac_bus_queue_event(struct hdac_bus *bus, u32 res, u32 res_ex);

int snd_hdac_bus_add_device(struct hdac_bus *bus, struct hdac_device *codec);
void snd_hdac_bus_remove_device(struct hdac_bus *bus,
				struct hdac_device *codec);

static inline void snd_hdac_codec_link_up(struct hdac_device *codec)
{
	set_bit(codec->addr, &codec->bus->codec_powered);
}

static inline void snd_hdac_codec_link_down(struct hdac_device *codec)
{
	clear_bit(codec->addr, &codec->bus->codec_powered);
}

int snd_hdac_bus_send_cmd(struct hdac_bus *bus, unsigned int val);
int snd_hdac_bus_get_response(struct hdac_bus *bus, unsigned int addr,
			      unsigned int *res);

bool snd_hdac_bus_init_chip(struct hdac_bus *bus, bool full_reset);
void snd_hdac_bus_stop_chip(struct hdac_bus *bus);
void snd_hdac_bus_init_cmd_io(struct hdac_bus *bus);
void snd_hdac_bus_stop_cmd_io(struct hdac_bus *bus);
void snd_hdac_bus_enter_link_reset(struct hdac_bus *bus);
void snd_hdac_bus_exit_link_reset(struct hdac_bus *bus);

void snd_hdac_bus_update_rirb(struct hdac_bus *bus);
void snd_hdac_bus_handle_stream_irq(struct hdac_bus *bus, unsigned int status,
				    void (*ack)(struct hdac_bus *,
						struct hdac_stream *));

/*
 * macros for easy use
 */
#define _snd_hdac_chip_write(type, chip, reg, value) \
	((chip)->io_ops->reg_write ## type(value, (chip)->remap_addr + (reg)))
#define _snd_hdac_chip_read(type, chip, reg) \
	((chip)->io_ops->reg_read ## type((chip)->remap_addr + (reg)))

/* read/write a register, pass without AZX_REG_ prefix */
#define snd_hdac_chip_writel(chip, reg, value) \
	_snd_hdac_chip_write(l, chip, AZX_REG_ ## reg, value)
#define snd_hdac_chip_writew(chip, reg, value) \
	_snd_hdac_chip_write(w, chip, AZX_REG_ ## reg, value)
#define snd_hdac_chip_writeb(chip, reg, value) \
	_snd_hdac_chip_write(b, chip, AZX_REG_ ## reg, value)
#define snd_hdac_chip_readl(chip, reg) \
	_snd_hdac_chip_read(l, chip, AZX_REG_ ## reg)
#define snd_hdac_chip_readw(chip, reg) \
	_snd_hdac_chip_read(w, chip, AZX_REG_ ## reg)
#define snd_hdac_chip_readb(chip, reg) \
	_snd_hdac_chip_read(b, chip, AZX_REG_ ## reg)

/* update a register, pass without AZX_REG_ prefix */
#define snd_hdac_chip_updatel(chip, reg, mask, val) \
	snd_hdac_chip_writel(chip, reg, \
			     (snd_hdac_chip_readl(chip, reg) & ~(mask)) | (val))
#define snd_hdac_chip_updatew(chip, reg, mask, val) \
	snd_hdac_chip_writew(chip, reg, \
			     (snd_hdac_chip_readw(chip, reg) & ~(mask)) | (val))
#define snd_hdac_chip_updateb(chip, reg, mask, val) \
	snd_hdac_chip_writeb(chip, reg, \
			     (snd_hdac_chip_readb(chip, reg) & ~(mask)) | (val))

/*
 * HD-audio stream
 */
struct hdac_stream {
	struct hdac_bus *bus;
	struct snd_dma_buffer bdl; /* BDL buffer */
	__le32 *posbuf;		/* position buffer pointer */
	int direction;		/* playback / capture (SNDRV_PCM_STREAM_*) */

	unsigned int bufsize;	/* size of the play buffer in bytes */
	unsigned int period_bytes; /* size of the period in bytes */
	unsigned int frags;	/* number for period in the play buffer */
	unsigned int fifo_size;	/* FIFO size */

	void __iomem *sd_addr;	/* stream descriptor pointer */

	u32 sd_int_sta_mask;	/* stream int status mask */

	/* pcm support */
	struct snd_pcm_substream *substream;	/* assigned substream,
						 * set in PCM open
						 */
	unsigned int format_val;	/* format value to be set in the
					 * controller and the codec
					 */
	unsigned char stream_tag;	/* assigned stream */
	unsigned char index;		/* stream index */
	int assigned_key;		/* last device# key assigned to */

	bool opened:1;
	bool running:1;
	bool no_period_wakeup:1;

	/* timestamp */
	unsigned long start_wallclk;	/* start + minimum wallclk */
	unsigned long period_wallclk;	/* wallclk for period */
	struct timecounter  tc;
	struct cyclecounter cc;
	int delay_negative_threshold;

	struct list_head list;
};

void snd_hdac_stream_init(struct hdac_bus *bus, struct hdac_stream *azx_dev,
			  int idx, int direction, int tag);
struct hdac_stream *snd_hdac_stream_assign(struct hdac_bus *bus,
					   struct snd_pcm_substream *substream);
void snd_hdac_stream_release(struct hdac_stream *azx_dev);

int snd_hdac_stream_setup(struct hdac_stream *azx_dev);
void snd_hdac_stream_cleanup(struct hdac_stream *azx_dev);
int snd_hdac_stream_setup_periods(struct hdac_stream *azx_dev);
void snd_hdac_stream_start(struct hdac_stream *azx_dev, bool fresh_start);
void snd_hdac_stream_clear(struct hdac_stream *azx_dev);
void snd_hdac_stream_stop(struct hdac_stream *azx_dev);
void snd_hdac_stream_reset(struct hdac_stream *azx_dev);
void snd_hdac_stream_sync_trigger(struct hdac_stream *azx_dev, bool set,
				  unsigned int streams, unsigned int reg);
void snd_hdac_stream_sync(struct hdac_stream *azx_dev, bool start,
			  unsigned int streams);
void snd_hdac_stream_timecounter_init(struct hdac_stream *azx_dev,
				      unsigned int streams);
/*
 * macros for easy use
 */
#define _snd_hdac_stream_write(type, dev, reg, value)			\
	((dev)->bus->io_ops->reg_write ## type(value, (dev)->sd_addr + (reg)))
#define _snd_hdac_stream_read(type, dev, reg)				\
	((dev)->bus->io_ops->reg_read ## type((dev)->sd_addr + (reg)))

/* read/write a register, pass without AZX_REG_ prefix */
#define snd_hdac_stream_writel(dev, reg, value) \
	_snd_hdac_stream_write(l, dev, AZX_REG_ ## reg, value)
#define snd_hdac_stream_writew(dev, reg, value) \
	_snd_hdac_stream_write(w, dev, AZX_REG_ ## reg, value)
#define snd_hdac_stream_writeb(dev, reg, value) \
	_snd_hdac_stream_write(b, dev, AZX_REG_ ## reg, value)
#define snd_hdac_stream_readl(dev, reg) \
	_snd_hdac_stream_read(l, dev, AZX_REG_ ## reg)
#define snd_hdac_stream_readw(dev, reg) \
	_snd_hdac_stream_read(w, dev, AZX_REG_ ## reg)
#define snd_hdac_stream_readb(dev, reg) \
	_snd_hdac_stream_read(b, dev, AZX_REG_ ## reg)

/* update a register, pass without AZX_REG_ prefix */
#define snd_hdac_stream_updatel(dev, reg, mask, val) \
	snd_hdac_stream_writel(dev, reg, \
			       (snd_hdac_stream_readl(dev, reg) & \
				~(mask)) | (val))
#define snd_hdac_stream_updatew(dev, reg, mask, val) \
	snd_hdac_stream_writew(dev, reg, \
			       (snd_hdac_stream_readw(dev, reg) & \
				~(mask)) | (val))
#define snd_hdac_stream_updateb(dev, reg, mask, val) \
	snd_hdac_stream_writeb(dev, reg, \
			       (snd_hdac_stream_readb(dev, reg) & \
				~(mask)) | (val))

/*
 * generic array helpers
 */
void *snd_array_new(struct snd_array *array);
void snd_array_free(struct snd_array *array);
static inline void snd_array_init(struct snd_array *array, unsigned int size,
				  unsigned int align)
{
	array->elem_size = size;
	array->alloc_align = align;
}

static inline void *snd_array_elem(struct snd_array *array, unsigned int idx)
{
	return array->list + idx * array->elem_size;
}

static inline unsigned int snd_array_index(struct snd_array *array, void *ptr)
{
	return (unsigned long)(ptr - array->list) / array->elem_size;
}

#endif /* __SOUND_HDAUDIO_H */
