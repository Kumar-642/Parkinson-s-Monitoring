library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity vga_centric is
    Port (
        clk25       : in  STD_LOGIC;
        vga_red     : out STD_LOGIC_VECTOR(3 downto 0);
        vga_green   : out STD_LOGIC_VECTOR(3 downto 0);
        vga_blue    : out STD_LOGIC_VECTOR(3 downto 0);
        vga_hsync   : out STD_LOGIC;
        vga_vsync   : out STD_LOGIC;
        frame_addr  : out STD_LOGIC_VECTOR(16 downto 0); -- 17-bit for 76,800 depth
        frame_pixel : in  STD_LOGIC_VECTOR(11 downto 0)
    );
end vga_centric;

architecture Behavioral of vga_centric is
    -- VGA Timing remains 640x480 to keep the monitor happy
    constant hRez : natural := 640;
    constant vRez : natural := 480;
    constant hMaxCount : natural := 800;
    constant vMaxCount : natural := 525;

    -- Box Coordinates (Relative to the 640x480 screen)
    -- This draws a 256x256 box in the center
    constant BOX_START_X : natural := 192;
    constant BOX_END_X   : natural := 447;
    constant BOX_START_Y : natural := 112;
    constant BOX_END_Y   : natural := 367;

    signal hCounter : unsigned(9 downto 0) := (others => '0');
    signal vCounter : unsigned(9 downto 0) := (others => '0');
    signal address  : unsigned(16 downto 0) := (others => '0');

begin

    -- Output the 17-bit address
    frame_addr <= std_logic_vector(address);

    process(clk25)
    begin
        if rising_edge(clk25) then
            -- 1. Standard VGA Counters
            if hCounter = hMaxCount-1 then
                hCounter <= (others => '0');
                if vCounter = vMaxCount-1 then
                    vCounter <= (others => '0');
                else
                    vCounter <= vCounter + 1;
                end if;
            else
                hCounter <= hCounter + 1;
            end if;

            -- 2. PIXEL DOUBLING ADDRESS LOGIC
            -- We divide hCounter and vCounter by 2 (shift right)
            -- to map a 320x240 image to a 640x480 screen.
            -- Address = (vCounter/2 * 320) + (hCounter/2)
            if vCounter < vRez and hCounter < hRez then
                address <= to_unsigned((to_integer(vCounter/2) * 320) + to_integer(hCounter/2), 17);
            else
                address <= (others => '0');
            end if;

            -- 3. COLOR & BOX LOGIC
            if vCounter < vRez and hCounter < hRez then
                -- Check for 256x256 Box Boundary
                if ((hCounter = BOX_START_X or hCounter = BOX_END_X) and (vCounter >= BOX_START_Y and vCounter <= BOX_END_Y)) or
                   ((vCounter = BOX_START_Y or vCounter = BOX_END_Y) and (hCounter >= BOX_START_X and hCounter <= BOX_END_X)) then
                    vga_red   <= "1111";
                    vga_green <= "1111";
                    vga_blue  <= "1111";
                else
                    vga_red   <= frame_pixel(11 downto 8);
                    vga_green <= frame_pixel(7 downto 4);
                    vga_blue  <= frame_pixel(3 downto 0);
                end if;
            else
                vga_red <= "0000"; vga_green <= "0000"; vga_blue <= "0000";
            end if;

            -- 4. SYNC GENERATION
            if hCounter > 656 and hCounter <= 752 then vga_hsync <= '0'; else vga_hsync <= '1'; end if;
            if vCounter >= 490 and vCounter < 492 then vga_vsync <= '0'; else vga_vsync <= '1'; end if;

        end if;
    end process;
end Behavioral;

