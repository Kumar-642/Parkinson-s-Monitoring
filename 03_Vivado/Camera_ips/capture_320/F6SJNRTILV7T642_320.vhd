library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity ov7670_capture_320 is
    Port (
           pclk        : in  STD_LOGIC;
           vsync       : in  STD_LOGIC;
           href        : in  STD_LOGIC;
           d           : in  STD_LOGIC_VECTOR (7 downto 0);

           snapshot_en : in  STD_LOGIC;  -- From AXI GPIO

           -- Address (same for both BRAMs)
           addr        : out STD_LOGIC_VECTOR (16 downto 0); -- 17 bits (76800 max)

           -- Pixel output
           dout        : out STD_LOGIC_VECTOR (11 downto 0);

           -- Dual write enables
           we_live     : out STD_LOGIC;  -- For VGA BRAM
           we_snapshot : out STD_LOGIC   -- For Snapshot BRAM
         );
end ov7670_capture_320;

architecture Behavioral of ov7670_capture_320 is

   signal d_latch : std_logic_vector(15 downto 0) := (others => '0');
   signal wr_hold : std_logic_vector(1 downto 0)  := (others => '0');

   signal x : unsigned(9 downto 0) := (others => '0'); -- 0..639
   signal y : unsigned(9 downto 0) := (others => '0'); -- 0..479

   signal address_next : unsigned(16 downto 0) := (others => '0');

begin

   addr <= std_logic_vector(address_next);

   process(pclk)
   begin
      if rising_edge(pclk) then

         ------------------------------------------------------------
         -- Reset at start of frame
         ------------------------------------------------------------
         if vsync = '1' then
            x <= (others => '0');
            y <= (others => '0');
            address_next <= (others => '0');
            wr_hold <= (others => '0');
            we_live <= '0';
            we_snapshot <= '0';

         else

            wr_hold <= wr_hold(0) & (href and not wr_hold(0));
            d_latch <= d_latch(7 downto 0) & d;

            if wr_hold(1) = '1' then

               -----------------------------------------------------
               -- RGB565 → RGB444
               -----------------------------------------------------
               dout <= d_latch(10 downto 7) &
                       d_latch(15 downto 12) &
                       d_latch(4 downto 1);

               -----------------------------------------------------
               -- Downsample: store only even pixel & even line
               -----------------------------------------------------
               if (x(0) = '0' and y(0) = '0') then

                  -- LIVE BRAM always writes
                  we_live <= '1';

                  -- SNAPSHOT BRAM writes only when enabled
                  if snapshot_en = '1' then
                     we_snapshot <= '1';
                  else
                     we_snapshot <= '0';
                  end if;

                  address_next <= address_next + 1;

               else
                  we_live <= '0';
                  we_snapshot <= '0';
               end if;

               -----------------------------------------------------
               -- Pixel Counters
               -----------------------------------------------------
               if x = 639 then
                  x <= (others => '0');
                  y <= y + 1;
               else
                  x <= x + 1;
               end if;

            else
               we_live <= '0';
               we_snapshot <= '0';
            end if;

         end if;

      end if;
   end process;

end Behavioral;
